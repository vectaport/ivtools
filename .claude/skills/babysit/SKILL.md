# Babysitting PRs on this repo

Repo-specific addition to the general PR-babysitting rules: read this file
before acting on a CI or review event on a PR you opened or are driving.

## Comment before you push

When responding to a review finding (Greptile or otherwise) that needs a
code fix: reply to the finding first, then push the commit that fixes it
-- not the other way around. A push to the PR head kicks off a fresh
review pass immediately. Pushing before the reply risks that new pass
re-billing or running before your explanation of the finding it answers
is actually attached to the thread.

Order for each finding: verify it, reply explaining what you're doing (or
why you're not), then push.
