#!/bin/bash
set -e
echo "-- Generating documentation."
echo "-- Node version: $(node -v)"
echo "-- NPM version: $(npm -v)"

git fetch origin
git checkout ${CHECKOUT_REF/refs\/heads\//}

cd docs
npm install
npm run build

echo "-- Documentation successfully generated."

if git diff --quiet; then
	echo "-- No documentation changes to commit."
	exit 0
fi

REMOTE_URL="$(git config remote.origin.url)"
TARGET_REPO=${REMOTE_URL/https:\/\/github.com\//github.com/}
GITHUB_REPO=https://${GH_TOKEN:-git}@${TARGET_REPO}

git config user.name "Azure CI"
git config user.email "$COMMIT_AUTHOR_EMAIL"

git add ./generated
git pull
git commit -m "docs(ci): Update protocol.md - $(git rev-parse --short HEAD) [skip ci]"
git push -q $GITHUB_REPO
