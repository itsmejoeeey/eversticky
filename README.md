# EverSticky

<img src="https://i.imgur.com/KrAfarG.png" width="400">

A Linux open-source sticky note client that syncs to Evernote 

*(early alpha)*

## Installation and running
| Binaries            |
| ------------------- |
| Coming soon...      |

Install instructions coming soon...

### Compiling from source
Dependencies:
* Qt *>= 5*
* libxpm / libxpm-dev

> IMPORTANT NOTE: You will need a production Evernote API key to compile the application and be able to see and alter notes you currently store on Evernote. This secret (along with the desired domain and key) needs to be provided at the top of `./src/eversticky.pro`.

From the root of the repo run the following commands:
```
mkdir src/build
cd src/build
qmake ../eversticky.pro
make
```
This produces the executable binary `./eversticky` in the current directory.

## TODO
Feel free to tackle one and make a PR ;))

- [ ] Provide binaries and instructions for popular distributions
- [ ] Change the `./eversticky.pro` file so build paths, desktop files, etc are correct and managed automatically
- [ ] Implement a proper version numbering scheme
- [ ] Fix `QSqlQuery::value: not positioned on a valid record` errors on login
- [ ] Ensure duplicate entries are not being added to the `notes` table in the cache database
- [ ] Provide a settings pane that allows the user to set sync interval and 'start on startup' state
- [ ] Implement more robust handling of note formatting when syncing to/from Evernote. Currently unexpected behaviour (such as inserting numerous newlines) is experienced when non-plain text styles are in use.
- [ ] Ensure it is impossible for the code to overflow
- [ ] Support for Windows/Mac OSX

## Bugs
This project is still in its early days and bugs are expected (some are known above in the TODO section). Don't hesitate to open an issue if you encounter some unexpected behaviour.

## License
`src/qevercloud` licensed under MIT terms. Consists of `v4.0.0` release from @d1vanov's fork (https://github.com/d1vanov/QEverCloud)

For all other code see `LICENSE`
