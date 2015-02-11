
Readme.md
__________

git_updater is a tool which runs in the desktop background and keeps
the local repositories updated with the remotes. The total data is brought down
to the local storage and all references are updated. **The noticed changes are
but not merged with the local repository**.

The repository started with my use to keep the master branch of local remote
updated with the master branch of the upstream remote. This is under my style
that all changes are to be made in the **dev branch** by the maintainer.

The idea has currently moved from that thought to fetching all remotes and later
using command line tools to merge changes when needed.

This can particularly involve making a full-blown GUI for repository viewing
which compares the references whether local or remote.

To run this as a daemon, copy the file com.shkesar.gitud.plist in /Library/LaunchAgents/ path. Add the path of the compiled binary below the program key.
The daemon will be executed on user login.
