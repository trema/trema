#! /bin/sh

sqlite3 authorized_host.db 'create table authorized_host (mac unsigned bigint, description text)'
