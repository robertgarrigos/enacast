Enacast
========
Backdrop cms module to show podcast players from Enacast podcast service.

This module exposes new field types to show last podcast for a given
program or a given podcast in a node. It also creates a new block for actual
streaming of a given radio station.


Installation
------------

- Install this module using the official Backdrop CMS instructions at
  https://docs.backdropcms.org/documentation/extend-with-modules.

- Visit the configuration page under Administration > Configuration > Services >
  Enacast (admin/config/services/enacast) and enter the required information.


Documentation
-------------
The new field "Enacast program" enables you to format that field as a player
for the latest podcast of that program.

The new field "Enacast podcast pk id" enables you to format that fiels as a
player for that given podcast.

There is a new field "Radio" which is not being used right now. In a new future
would allow to list programes by a given radio station.

Issues
------

Bugs and Feature Requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/enacast/issues.


Current Maintainers
-------------------

- [Robert Garrigós](https://github.com/robertgarrigos)


Credits
-------
- Originally written for Backdrop by [Robert Garrigós](https://github.com/robertgarrigos).

License
-------

This project is GPL v2 software.
See the LICENSE.txt file in this directory for complete text.
