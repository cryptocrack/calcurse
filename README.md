# calcurse

[![Build and test](https://github.com/lfos/calcurse/actions/workflows/make.yml/badge.svg)](https://github.com/lfos/calcurse/actions/workflows/make.yml)
[![Lint Python](https://github.com/lfos/calcurse/actions/workflows/lint_python.yml/badge.svg)](https://github.com/lfos/calcurse/actions/workflows/lint_python.yml)

![Demo](https://calcurse.org/images/demo.gif)

## Installation

calcurse is available on a number of Linux distributions:

[![Packaging status](https://repology.org/badge/vertical-allrepos/calcurse.svg?columns=3)](https://repology.org/project/calcurse/versions)

## Building from Source

Install the following build dependencies. If your distribution segments
development files from core packages (i.e., \*-devel or \*-dev packages), you
may need to install those as well:

* gcc
* autoconf-archive
* automake
* autopoint
* asciidoc
* gettext with development files
* ncurses with development files

If you are using a release tarball, the following commands can be used to build
and install calcurse:

    $ ./configure
    $ make
    $ make install

Note that `make install` needs to be run as root. When working on a Git
checkout, you need to run `./autogen.sh` before `./configure`.

## Source Overview

* `build-aux`: auxiliary files for the build process
* `contrib`: useful tools such as hooks or the CalDAV synchronization script
* `doc`: detailed documentation in plain text and HTML
* `po`: translations and i18n-related files
* `scripts`: additional official scripts, such as `calcurse-upgrade`
* `src`: the actual calcurse source files
* `test`: test suite and test cases for calcurse

## Authors

calcurse was created by Frederic Culot in 2004. Since 2011, the project is
maintained by Lukas Fleischer. Many core features added to calcurse since 2017
were designed and implemented by Lars Henriksen.

Of course, there are numerous other contributors. Check the Git commit log and
the `Thanks` section in the manual for a list of people who have contributed by
reporting bugs, sending fixes, or suggesting improvements.

## Contributing and Donations

Patches, bug reports and other requests are always welcome! You can submit them
to one of our mailing lists (check the [patch submission
guidelines](doc/submitting-patches.txt) for details) or via GitHub.

We are also extremely grateful for donations which help us continue developing
calcurse as open source software and are used to cover recurring costs, such as
for our servers. You can use https://calcurse.org/donate/ for a one-time
payment. If you prefer another form of donation, do not hesitate to contact us!
