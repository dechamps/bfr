#!/bin/bash

pushd build/html
rsync -avz -e ssh * joelbender,bfr@web.sourceforge.net:htdocs/
popd
