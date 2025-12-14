#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TurntableMIDIProcessor::TurntableMIDIProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      )
{
    // Initialize scale
    updateScaleNotes();

    // Start with a simple pentatonic melody pattern
    addDot(0.0f, 0, juce::Colour(0xffff6b35));      // Root
    addDot(90.0f, 2, juce::Colour(0xffff6b35));     // 3rd note
    addDot(180.0f, 4, juce::Colour(0xffff6b35));    // 5th note
    addDot(270.0f, 2, juce::Colour(0xffff6b35));    // 3rd note
}

TurntableMIDIProcessor::~TurntableMIDIProcessor()
{
}

//==============================================================================
const juce::String TurntableMIDIProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TurntableMIDIProcessor::acceptsMidi() const
{
    return true;  // Must accept MIDI for Ableton to recognize as Instrument
}

bool TurntableMIDIProcessor::producesMidi() const
{
    return true;
}

bool TurntableMIDIProcessor::isMidiEffect() const
{
    return false;  // Must be false for Ableton Live compatibility
}

double TurntableMIDIProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TurntableMIDIProcessor::getNumPrograms()
{
    return 1;
}

int TurntableMIDIProcessor::getCurrentProgram()
{
    return 0;
}

void TurntableMIDIProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String TurntableMIDIProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void TurntableMIDIProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void TurntableMIDIProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    triggeredThisRotation.resize(dots.size(), false);
}

void TurntableMIDIProcessor::releaseResources()
{
}

bool TurntableMIDIProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void TurntableMIDIProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    buffer.clear();

    // Process active note-offs first (notes that should end in this buffer)
    std::vector<ActiveNote> notesToKeep;
    for (const auto& note : activeNotes)
    {
        juce::int64 noteOffInBuffer = note.noteOffSample - totalSamplesProcessed;

        if (noteOffInBuffer < buffer.getNumSamples() && noteOffInBuffer >= 0)
        {
            // Note-off occurs in this buffer
            juce::MidiMessage noteOff = juce::MidiMessage::noteOff(note.channel, note.midiNote, (juce::uint8) 0);
            midiMessages.addEvent(noteOff, static_cast<int>(noteOffInBuffer));
        }
        else if (noteOffInBuffer >= buffer.getNumSamples())
        {
            // Note continues into future buffers
            notesToKeep.push_back(note);
        }
        // If noteOffInBuffer < 0, note already ended - discard it
    }
    activeNotes = notesToKeep;

    // Send any queued preview notes
    {
        juce::ScopedLock lock(previewNotesLock);
        for (const auto& previewNote : previewNotesToSend)
        {
            juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, previewNote.midiNote, (juce::uint8) 100);
            midiMessages.addEvent(noteOn, 0);

            // Schedule note-off for preview (100ms)
            juce::int64 noteOffSample = totalSamplesProcessed + static_cast<juce::int64>(sampleRate * 0.1);
            activeNotes.push_back({previewNote.midiNote, 1, noteOffSample});
        }
        previewNotesToSend.clear();
    }

    // Determine if we're playing and what BPM to use
    bool shouldPlay = isPlayingStandalone;  // Default to standalone state
    double currentBPM = standaloneBPM;

    // Get BPM and play state from host (overrides standalone if available)
    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (positionInfo->getBpm().hasValue())
            {
                currentBPM = *positionInfo->getBpm();
            }

            // If host is providing play state, use it
            if (positionInfo->getIsPlaying())
            {
                shouldPlay = true;
            }
        }
    }

    // Record player-style motor control: ramp up/down speed
    const float rampUpRate = 3.0f;    // Fast start (reaches full speed in ~0.33 seconds)
    const float rampDownRate = 0.4f;  // Gradual stop (comes to halt in ~2.5 seconds)
    const float rampStep = 1.0f / sampleRate;  // Per-sample increment

    if (motorRunning)
    {
        // Ramp up to full speed
        if (currentSpeedMultiplier < 1.0f)
            currentSpeedMultiplier = juce::jmin(1.0f, currentSpeedMultiplier + (rampUpRate * rampStep * buffer.getNumSamples()));
    }
    else
    {
        // Ramp down to stop
        if (currentSpeedMultiplier > 0.0f)
            currentSpeedMultiplier = juce::jmax(0.0f, currentSpeedMultiplier - (rampDownRate * rampStep * buffer.getNumSamples()));
    }

    // Scratching physics: apply friction to scratch velocity (like motor slowdown)
    if (!isBeingScratched && std::abs(scratchVelocity) > 0.01f)
    {
        // Apply friction to slow down the "thrown" turntable
        // Use same decay rate as motor ramp-down for consistent feel
        const float scratchDecayRate = 0.4f;  // Same as motor ramp down
        const float decayStep = scratchDecayRate / sampleRate;
        float velocityReduction = decayStep * buffer.getNumSamples();

        // Reduce velocity toward zero
        if (scratchVelocity > 0.0f)
            scratchVelocity = juce::jmax(0.0f, scratchVelocity - (std::abs(scratchVelocity) * velocityReduction));
        else
            scratchVelocity = juce::jmin(0.0f, scratchVelocity + (std::abs(scratchVelocity) * velocityReduction));

        // Stop completely if velocity is very small
        if (std::abs(scratchVelocity) < 0.1f)
            scratchVelocity = 0.0f;
    }

    // Apply scratch momentum (when not being actively scratched but has velocity)
    float previousRotation = currentRotation;
    if (!isBeingScratched && std::abs(scratchVelocity) > 0.01f)
    {
        // Apply scratch velocity
        float scratchIncrement = scratchVelocity * (buffer.getNumSamples() / sampleRate);
        currentRotation += scratchIncrement;

        // Wrap around and reset triggers when completing a rotation
        if (currentRotation < 0.0f)
        {
            currentRotation += 360.0f;
            // Reset trigger tracking when we complete a rotation (backward)
            std::fill(triggeredThisRotation.begin(), triggeredThisRotation.end(), false);
        }
        else if (currentRotation >= 360.0f)
        {
            currentRotation = std::fmod(currentRotation, 360.0f);
            // Reset trigger tracking when we complete a rotation (forward)
            std::fill(triggeredThisRotation.begin(), triggeredThisRotation.end(), false);
        }
    }
    // Only advance rotation if playing (or if motor is spinning down) and NOT being scratched or thrown
    // Only use motor rotation when scratch velocity is zero to avoid interference
    else if (!isBeingScratched && std::abs(scratchVelocity) < 0.01f && (shouldPlay || currentSpeedMultiplier > 0.0f))
    {
        // Calculate rotation increment based on BPM and speed
        // One full rotation per 2 bars (8 beats) at normal speed
        // Apply reverse if enabled
        double beatsPerSecond = currentBPM / 60.0;
        double effectiveSpeed = isReversed ? -speed : speed;
        effectiveSpeed *= currentSpeedMultiplier;  // Apply motor ramp
        double rotationsPerSecond = (beatsPerSecond / 8.0) * effectiveSpeed;
        double degreesPerSample = (rotationsPerSecond * 360.0) / sampleRate;
        float rotationIncrement = static_cast<float>(degreesPerSample * buffer.getNumSamples());

        currentRotation += rotationIncrement;

        // Wrap around and reset triggers when completing a rotation (both directions)
        if (currentRotation < 0.0f)
        {
            currentRotation += 360.0f;
            // Reset trigger tracking when we complete a rotation (backward/reverse)
            std::fill(triggeredThisRotation.begin(), triggeredThisRotation.end(), false);
        }
        else if (currentRotation >= 360.0f)
        {
            currentRotation = std::fmod(currentRotation, 360.0f);
            // Reset trigger tracking when we complete a rotation (forward)
            std::fill(triggeredThisRotation.begin(), triggeredThisRotation.end(), false);
        }
    }

    // Note triggering: Check each dot to see if we've crossed its angle
    // This works for normal playback, scratching, and scratch momentum
    if (previousRotation != currentRotation)
    {
        // Check each dot to see if we've crossed its angle
        for (size_t i = 0; i < dots.size(); ++i)
        {
            if (!dots[i].active)
                continue;

            // Calculate the trigger angle (when dot is at top/under sensor)
            // Visual angle 0° = top (sensor arm position)
            // Trigger when: (dot.angle - currentRotation) = 0°
            // Which means: currentRotation = dot.angle
            float triggerAngle = dots[i].angle;

            // Check if we've crossed the trigger angle (works for both directions)
            // Add small tolerance to handle floating-point precision and very small movements
            bool crossed = false;
            float rotationDelta = currentRotation - previousRotation;
            const float tolerance = 0.5f;  // 0.5 degree tolerance for edge cases

            // Handle wrap-around
            if (rotationDelta > 180.0f)
                rotationDelta -= 360.0f;
            else if (rotationDelta < -180.0f)
                rotationDelta += 360.0f;

            if (std::abs(rotationDelta) < 0.001f)
            {
                // No movement - skip crossing check
                continue;
            }
            else if (rotationDelta > 0.0f)
            {
                // Forward rotation (clockwise)
                float diff = triggerAngle - previousRotation;
                if (diff < 0.0f) diff += 360.0f;
                // Add tolerance: allow trigger if within range + tolerance
                crossed = (diff >= -tolerance && diff <= rotationDelta + tolerance);
            }
            else if (rotationDelta < 0.0f)
            {
                // Reverse rotation (counter-clockwise)
                float diff = previousRotation - triggerAngle;
                if (diff < 0.0f) diff += 360.0f;
                // Add tolerance: allow trigger if within range + tolerance
                crossed = (diff >= -tolerance && diff <= -rotationDelta + tolerance);
            }

            // Trigger MIDI note if we crossed this dot and haven't triggered it yet
            if (crossed && !triggeredThisRotation[i])
            {
                // Apply probability - check if this note should trigger
                float probRoll = random.nextFloat() * 100.0f;
                bool passedProbability = probRoll <= probability;

                if (!passedProbability)
                {
                    // Track the dot pass but mark as not triggered for visual feedback
                    {
                        juce::ScopedLock lock(triggeredDotsLock);
                        auto currentTime = juce::Time::currentTimeMillis();
                        recentlyTriggeredDots.push_back({
                            static_cast<int>(i),
                            currentTime,
                            0,              // velocity (not used when not triggered)
                            0.0f,           // gateTimeMs (not used when not triggered)
                            false,          // wasTriggered = false
                            swingBeatCounter
                        });

                        // Clean up old entries (older than 1000ms to accommodate gate times)
                        recentlyTriggeredDots.erase(
                            std::remove_if(recentlyTriggeredDots.begin(), recentlyTriggeredDots.end(),
                                [currentTime](const TriggeredDotInfo& entry) {
                                    return (currentTime - entry.timestamp) > 1000;
                                }),
                            recentlyTriggeredDots.end()
                        );
                    }

                    triggeredThisRotation[i] = true; // Mark as triggered even if skipped
                    continue; // Skip this note
                }

                // Calculate exactly when in the block the crossing occurred
                int triggerSample = 0;

                if (previousRotation < currentRotation)
                {
                    // Normal case: calculate how far through the block the crossing happened
                    float rotationRange = currentRotation - previousRotation;
                    float rotationToCrossing = triggerAngle - previousRotation;

                    if (rotationRange > 0.0f)
                    {
                        float fraction = rotationToCrossing / rotationRange;
                        triggerSample = static_cast<int>(fraction * buffer.getNumSamples());
                        triggerSample = juce::jlimit(0, buffer.getNumSamples() - 1, triggerSample);
                    }
                }
                // For wrap-around case, trigger at start of block for simplicity

                // Apply swing timing based on beat position in rotation
                // One full rotation = 8 beats, so calculate which beat this note falls on
                swingBeatCounter++;
                if (swing > 0.0f)
                {
                    // Calculate which 16th note subdivision this trigger falls on (0-31)
                    // One rotation = 8 beats = 32 sixteenth notes
                    float rotationProgress = currentRotation / 360.0f;  // 0.0 to 1.0
                    int sixteenthNote = static_cast<int>(rotationProgress * 32.0f) % 32;

                    // Apply swing to every other 16th note (odd numbered ones)
                    // This creates the classic "long-short" swing pattern
                    if (sixteenthNote % 2 == 1)
                    {
                        // Calculate tempo-relative swing delay
                        // At 100% swing: delay by full 16th note (dramatic swing)
                        // At 66% swing: classic jazz triplet feel
                        // At 50% swing: no swing (straight)
                        double secondsPerBeat = 60.0 / currentBPM;
                        double sixteenthNoteDuration = secondsPerBeat / 4.0;  // 16th note subdivision

                        // Swing percentage maps to delay amount:
                        // 50% = no delay (straight), 66% = triplet feel, 100% = full 16th delay
                        float swingRatio = (swing / 100.0f);  // 0.0 to 1.0
                        float delayRatio = (swingRatio - 0.5f) * 2.0f;  // -1.0 to 1.0, centered at 0.5 (50%)
                        delayRatio = juce::jlimit(0.0f, 1.0f, delayRatio);  // Clamp to 0.0-1.0

                        double swingDelaySec = sixteenthNoteDuration * delayRatio;
                        int swingOffset = static_cast<int>(swingDelaySec * sampleRate);
                        triggerSample = juce::jmin(buffer.getNumSamples() - 1, triggerSample + swingOffset);
                    }
                }

                // Get MIDI note from ring index based on current scale
                int midiNote = ringToMidiNote(dots[i].ringIndex);

                // Calculate velocity with variation
                int finalVelocity = globalVelocity;
                if (velocityVariation > 0.0f)
                {
                    // Add random variation based on velocityVariation parameter
                    float variation = (random.nextFloat() * 2.0f - 1.0f) * (velocityVariation / 100.0f);
                    finalVelocity = static_cast<int>(globalVelocity * (1.0f + variation * 0.5f));
                    finalVelocity = juce::jlimit(1, 127, finalVelocity);
                }

                // All notes go to MIDI channel 1
                juce::MidiMessage noteOn = juce::MidiMessage::noteOn(
                    1,  // Channel 1
                    midiNote,
                    (juce::uint8) finalVelocity  // Use calculated velocity
                );
                midiMessages.addEvent(noteOn, triggerSample);

                // Schedule note-off based on gate time (will be sent in future buffer)
                juce::int64 absoluteNoteOffSample = totalSamplesProcessed + triggerSample +
                    static_cast<juce::int64>(sampleRate * (gateTimeMs / 1000.0));
                activeNotes.push_back({midiNote, 1, absoluteNoteOffSample});

                triggeredThisRotation[i] = true;

                // Track this dot for visual feedback with full parameter info
                {
                    juce::ScopedLock lock(triggeredDotsLock);
                    auto currentTime = juce::Time::currentTimeMillis();
                    recentlyTriggeredDots.push_back({
                        static_cast<int>(i),
                        currentTime,
                        finalVelocity,      // Actual velocity after variation
                        gateTimeMs,         // Gate time parameter
                        true,               // wasTriggered = true
                        swingBeatCounter    // Beat counter for swing visualization
                    });

                    // Clean up old entries (older than 1000ms to accommodate gate times)
                    recentlyTriggeredDots.erase(
                        std::remove_if(recentlyTriggeredDots.begin(), recentlyTriggeredDots.end(),
                            [currentTime](const TriggeredDotInfo& entry) {
                                return (currentTime - entry.timestamp) > 1000;
                            }),
                        recentlyTriggeredDots.end()
                    );
                }
            }
        }
    }

    // Increment total samples processed for accurate note-off timing across buffers
    totalSamplesProcessed += buffer.getNumSamples();
}

//==============================================================================
bool TurntableMIDIProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TurntableMIDIProcessor::createEditor()
{
    return new TurntableMIDIEditor (*this);
}

//==============================================================================
void TurntableMIDIProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save plugin state (dots configuration, speed, scale, etc.)
    juce::MemoryOutputStream stream(destData, false);

    stream.writeFloat(speed);
    stream.writeInt(static_cast<int>(currentScale));
    stream.writeInt(rootNote);
    stream.writeInt(static_cast<int>(dots.size()));

    for (const auto& dot : dots)
    {
        stream.writeFloat(dot.angle);
        stream.writeInt(dot.ringIndex);
        stream.writeInt(dot.color.getARGB());
        stream.writeBool(dot.active);
    }

    // Save new parameters
    stream.writeInt(globalVelocity);
    stream.writeFloat(gateTimeMs);
    stream.writeBool(isReversed);
    stream.writeFloat(probability);
    stream.writeFloat(velocityVariation);
    stream.writeFloat(swing);
}

void TurntableMIDIProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore plugin state
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);

    speed = stream.readFloat();
    currentScale = static_cast<ScaleType>(stream.readInt());
    rootNote = stream.readInt();
    updateScaleNotes();

    int numDots = stream.readInt();

    dots.clear();
    for (int i = 0; i < numDots; ++i)
    {
        TurntableDot dot;
        dot.angle = stream.readFloat();
        dot.ringIndex = stream.readInt();
        dot.color = juce::Colour(stream.readInt());
        dot.active = stream.readBool();
        dots.push_back(dot);
    }

    triggeredThisRotation.resize(dots.size(), false);

    // Load new parameters (with defaults for older saved states)
    if (!stream.isExhausted())
    {
        globalVelocity = stream.readInt();
        gateTimeMs = stream.readFloat();
        isReversed = stream.readBool();
        probability = stream.readFloat();
        velocityVariation = stream.readFloat();
        swing = stream.readFloat();
    }
}

//==============================================================================
void TurntableMIDIProcessor::addDot(float angle, int ringIndex, juce::Colour color)
{
    TurntableDot dot;
    dot.angle = angle;
    dot.ringIndex = ringIndex;
    dot.color = color;
    dot.active = true;
    dots.push_back(dot);
    triggeredThisRotation.resize(dots.size(), false);
}

void TurntableMIDIProcessor::removeDot(int index)
{
    if (index >= 0 && index < static_cast<int>(dots.size()))
    {
        dots.erase(dots.begin() + index);
        triggeredThisRotation.resize(dots.size(), false);
    }
}

void TurntableMIDIProcessor::clearAllDots()
{
    dots.clear();
    triggeredThisRotation.clear();
}

//==============================================================================
// Scale system implementation

std::vector<int> TurntableMIDIProcessor::getScaleIntervals(ScaleType scale)
{
    switch (scale)
    {
        case ScaleType::Major:           return {0, 2, 4, 5, 7, 9, 11, 12};
        case ScaleType::Minor:           return {0, 2, 3, 5, 7, 8, 10, 12};
        case ScaleType::HarmonicMinor:   return {0, 2, 3, 5, 7, 8, 11, 12};
        case ScaleType::MelodicMinor:    return {0, 2, 3, 5, 7, 9, 11, 12};
        case ScaleType::Pentatonic:      return {0, 2, 4, 7, 9, 12};
        case ScaleType::PentatonicMinor: return {0, 3, 5, 7, 10, 12};
        case ScaleType::Blues:           return {0, 3, 5, 6, 7, 10, 12};
        case ScaleType::Dorian:          return {0, 2, 3, 5, 7, 9, 10, 12};
        case ScaleType::Phrygian:        return {0, 1, 3, 5, 7, 8, 10, 12};
        case ScaleType::Lydian:          return {0, 2, 4, 6, 7, 9, 11, 12};
        case ScaleType::Mixolydian:      return {0, 2, 4, 5, 7, 9, 10, 12};
        case ScaleType::Locrian:         return {0, 1, 3, 5, 6, 8, 10, 12};
        case ScaleType::Chromatic:       return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        default:                         return {0, 2, 4, 7, 9, 12};
    }
}

void TurntableMIDIProcessor::updateScaleNotes()
{
    scaleNotes.clear();
    auto intervals = getScaleIntervals(currentScale);

    // Create single octave of the scale
    int baseMIDI = rootNote + (baseOctave * 12);
    for (int interval : intervals)
    {
        // Skip the octave repeat (12) to keep it to one octave
        if (interval == 12) continue;
        scaleNotes.push_back(baseMIDI + interval);
    }
}

void TurntableMIDIProcessor::setScale(ScaleType newScale)
{
    currentScale = newScale;
    updateScaleNotes();
}

void TurntableMIDIProcessor::setRootNote(int newRoot)
{
    rootNote = juce::jlimit(0, 11, newRoot);
    updateScaleNotes();
}

void TurntableMIDIProcessor::setOctaveShift(int shift)
{
    octaveShift = juce::jlimit(-2, 2, shift);
}

int TurntableMIDIProcessor::ringToMidiNote(int ringIndex) const
{
    if (ringIndex >= 0 && ringIndex < static_cast<int>(scaleNotes.size()))
    {
        int baseNote = scaleNotes[ringIndex];
        return baseNote + (octaveShift * 12); // Apply octave shift
    }
    return 60; // Default to middle C
}

//==============================================================================
// Preview note triggering
void TurntableMIDIProcessor::triggerPreviewNote(int ringIndex)
{
    int midiNote = ringToMidiNote(ringIndex);

    juce::ScopedLock lock(previewNotesLock);
    PreviewNote preview;
    preview.midiNote = midiNote;
    preview.timeStamp = 0;
    previewNotesToSend.push_back(preview);
}

//==============================================================================
// Get recently triggered dots for visual feedback
std::vector<TurntableMIDIProcessor::TriggeredDotInfo> TurntableMIDIProcessor::getRecentlyTriggeredDots() const
{
    juce::ScopedLock lock(triggeredDotsLock);

    // Return all recently triggered dots (cleanup happens in processBlock)
    return recentlyTriggeredDots;
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TurntableMIDIProcessor();
}
