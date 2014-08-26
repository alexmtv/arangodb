/*jslint indent: 2, nomen: true, maxlen: 120, sloppy: true, vars: true, white: true, plusplus: true, -W051: true */
/*global require, UPGRADE_ARGS: true, UPGRADE_STARTED: true, ArangoServerState */

////////////////////////////////////////////////////////////////////////////////
/// @brief initialise a new database
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                         initialise a new database
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief initialise a new database
////////////////////////////////////////////////////////////////////////////////

(function () {
  return function () {
    var internal = require("internal");
    var console = require("console");

    // run the local upgrade-database script (UPGRADE_ARGS has been set)
    var result = internal.loadStartup("server/upgrade-database.js");

    result = UPGRADE_STARTED && result;
    delete UPGRADE_STARTED;
    delete UPGRADE_ARGS;

    if (! result) {
      return false;
    }

    // statistics can be turned off
    if (internal.enableStatistics) {
      require("org/arangodb/statistics").startup();
    }

    // start the queue manager once
    require('org/arangodb/foxx/queues/manager').run();

    console.info("bootstraped DB server %s", ArangoServerState.id());
    return true;
  };
}());

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
