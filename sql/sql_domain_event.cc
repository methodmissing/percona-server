/*
   Copyright (c) 2005, 2022, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/sql_binlog.h"

#include <stddef.h>
#include <sys/types.h>
#include <utility>

#include "base64.h"  // base64_needed_decoded_length
#include "lex_string.h"
#include "libbinlogevents/include/binlog_event.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog_reader.h"
#include "sql/sql_domain_event.h"
#include "sql/log_event.h"  // Format_description_log_event
#include "sql/psi_memory_key.h"
#include "sql/rpl_info_factory.h"  // Rpl_info_factory
#include "sql/rpl_info_handler.h"
#include "sql/rpl_rli.h"  // Relay_log_info
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "sql/system_variables.h"

/**
  Execute a DOMAIN EVENT statement.

  TODO

  @param thd Pointer to THD object for the client thread executing the
  statement.
*/

int mysql_domain_event_statement(THD *thd, LEX_STRING name, LEX_STRING aggregate_type, LEX_STRING aggregate_id, LEX_STRING payload) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("domain event: '%*s'",
                      (int)(payload.length < 2048
                                ? payload.length
                                : 2048),
                      payload.str));

  if (name.length == 0) {
    my_error(ER_SYNTAX_ERROR, MYF(0));
    return 0;
  }

  if (aggregate_type.length == 0) {
    my_error(ER_SYNTAX_ERROR, MYF(0));
    return 0;
  }

  if (aggregate_id.length == 0) {
    my_error(ER_SYNTAX_ERROR, MYF(0));
    return 0;
  }

  if (payload.length == 0) {
    my_error(ER_SYNTAX_ERROR, MYF(0));
    return 0;
  }

  Domain_log_event ev(thd, name, aggregate_type, aggregate_id, payload);
  return mysql_bin_log.write_event(&ev);
}
