#!/bin/bash
# Helper script to sync (pull + push) Skald using PAT token
# Usage: ./git-sync.sh

TOKEN=$(cat .git/credentials 2>/dev/null | grep -o 'ghp_[^@]*')
REPO_URL="github.com/josephvolmer/skald.git"
BRANCH=$(git rev-parse --abbrev-ref HEAD)

if [ -z "$TOKEN" ]; then
    echo "❌ Error: No GitHub PAT token found in .git/credentials"
    echo "Please create .git/credentials with format:"
    echo "https://ghp_YOUR_TOKEN@github.com"
    exit 1
fi

echo "🔐 Configuring git remote with PAT..."
git remote set-url origin "https://${TOKEN}@${REPO_URL}"

echo "⬇️  Pulling from origin/${BRANCH}..."
git pull --rebase origin "$BRANCH"
PULL_EXIT=$?

if [ $PULL_EXIT -ne 0 ]; then
    echo "❌ Pull failed with exit code ${PULL_EXIT}"
    git remote set-url origin "https://${REPO_URL}"
    exit $PULL_EXIT
fi

echo "⬆️  Pushing to origin/${BRANCH}..."
git push origin "$BRANCH"
PUSH_EXIT=$?

echo "🧹 Cleaning up remote URL..."
git remote set-url origin "https://${REPO_URL}"

if [ $PUSH_EXIT -eq 0 ]; then
    echo "✅ Successfully synced ${BRANCH}!"
else
    echo "❌ Push failed with exit code ${PUSH_EXIT}"
fi

exit $PUSH_EXIT
