/*
 * Copyright (c) 2012 Balabit
 * Copyright (c) 2012 Gergely Nagy <algernon@balabit.hu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 */

#include "logproto-indented-multiline-server.h"
#include "messages.h"

static inline gboolean
log_proto_indented_multiline_is_continuation(guchar first_char)
{
  return (first_char == ' ' || first_char == '\t');
}

static gint
log_proto_indented_multiline_accumulate_line(LogProtoTextServer *s,
                                             const guchar *msg,
                                             gsize msg_len,
                                             gssize consumed_len)
{

  /* NOTES:
   *
   *    msg
   *      points to the beginning of the line _repeatedly_, e.g. as
   *      long as we return the we couldn't extract a message.
   *
   *    msg_len
   *      This is getting longer and longer as lines get accumulated
   *      in the message.
   *
   *    consumed_len
   *      Is the length of the message starting with "msg" that was already
   *      consumed by this function.  In practice this points to the EOL
   *      character of the last consumed line.
   *
   */
	/* First message (abcd): @ccaballero consumed_len = -1
	 * First message: @ccaballero msg_len = 4
	 * Second message (hola1234): consumed_len = 4
	 * Second message msg_len = 13
	 * Third message (datiosenosva): consumed_len = 8
	 * Third message msg_len = 21
	 */
  /* let's check if the current line is a continuation line or not */

	// TESTING @ccaballe COMO QUITAR EL BUCLE INFINITO???
	// TODO @ccaballe change if condition to be more readable
	msg_debug("[CC-logproto-indent] MESSAGE ES ", evt_tag_printf("msg", msg));


	if (consumed_len >= 0 && msg_len >= consumed_len + 1 && msg_len > 1
			) {
		guchar first_character_of_the_current_line = msg[consumed_len + 1];
		guchar second_character_of_the_current_line = msg[consumed_len];

		msg_debug("[CC-logproto-indent] MESSAGE LEN: ", evt_tag_int("message_len", msg_len));
		msg_debug("[CC-logproto-indent]: ", evt_tag_int("consumed_len", consumed_len));

//		msg_debug("[CC] MSG_STRING AFTER AFTER: ", evt_tag_printf("msg", msg[consumed_len + 2]));


		if (first_character_of_the_current_line == '\n') {
			if(second_character_of_the_current_line == '\n') {
				msg_debug("[CC-logproto-indent] HAY 22222222 SALTOS DE LINEA");
				// TESTING FOREVER
//  			  return LPT_CONSUME_LINE | LPT_WAITING;

			}
			else
				msg_debug("[CC-logproto-indent] HAY 111111111 SALTOS DE LINEA");

			msg_debug("[CC-logproto-indent] Processing line with handler");
			// TEST!!
			if (consumed_len == msg_len - 1) {
			  msg_debug("[CC-logproto-indent] Extraida porque consumed_len == msg_len - 1");
	          return LPT_REWIND_LINE | LPT_EXTRACTED;
			}
		}
	}
	// END TESTING

	if (consumed_len >= 0 && msg_len > consumed_len + 1)

    {
      guchar first_character_of_the_current_line = msg[consumed_len + 1];

      if (log_proto_indented_multiline_is_continuation(first_character_of_the_current_line))
        {
    	  msg_debug("[CC-logproto-indent] es continuacion, LPT_CONSUME_LINE | LPT_WAITING");
    	  return LPT_CONSUME_LINE | LPT_WAITING;
        }

      else
        {
		  msg_debug("[CC-logproto-indent] no hay saltoS DE LINEA");

		  // TESTING cconsumed len 0 llega cuando mandas el mensaje esperando una vez
		  if (consumed_len > 0) {
			  msg_debug("[CC-logproto-indent] SOLO SE ENVIA SI CONSUMED_LEN ES MAYOR 0");

			  return LPT_REWIND_LINE | LPT_EXTRACTED;
		  }
		  //		   END TESTING IF
        }
    }
  return LPT_CONSUME_LINE | LPT_WAITING;
}

void
log_proto_indented_multiline_server_init(LogProtoIMultiLineServer *self,
                                         LogTransport *transport,
                                         const LogProtoServerOptions *options)
{
  log_proto_text_server_init(&self->super, transport, options);
  self->super.accumulate_line = log_proto_indented_multiline_accumulate_line;
}

LogProtoServer *
log_proto_indented_multiline_server_new(LogTransport *transport, const LogProtoServerOptions *options)
{
  LogProtoIMultiLineServer *self = g_new0(LogProtoIMultiLineServer, 1);

  log_proto_indented_multiline_server_init(self, transport, options);
  return &self->super.super.super;
}
