#!/bin/bash
sudo date -u -s "`ssh -p 87 duncan@archbookpro.local 'date -u'`"
