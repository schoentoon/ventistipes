ventistipes
===========

[![Build Status](https://travis-ci.org/schoentoon/ventistipes.png)](https://travis-ci.org/schoentoon/ventistipes)

A SMTP server which pushes the received messages to a mobile device. This is for now just a work in progress, it is however functional.

Get started
===========

- Start off by creating a new database (PostgreSQL) using database_schema.sql.
- Edit include/config/dbinfo.h accordingly
- Follow the instructions in include/config/gcm_key.h
- Build it using $ make (run $ make clean first if you're building a newer version)
- Run it
- Actually fill in your database
- Send emails to it

You can find an example Android client over [here](https://github.com/schoentoon/ventistipes-android)

Database
========

The database schema currently has 2 tables, being allowed_in_mail and push_ids. allowed_in_mail will contain the email addresses that are allowed to send email to it. push_ids will contain the allowed recipients and their push information (note: 1 email address can have multiple entries). The push_type field in the push_ids table is purely for the future atm, currently the only valid number here is 0.

TODO
====

- Improve the SMTP parser
- Improve the sender/recipient control (think only a may email b)
- iOS/Window Phone support anyone?
