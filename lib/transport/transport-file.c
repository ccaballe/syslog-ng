/*
 * Copyright (c) 2002-2013 Balabit
 * Copyright (c) 1998-2013 Balázs Scheidler
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

#include "transport-file.h"

#include <errno.h>
#include <unistd.h>
#include "messages.h"

gssize
log_transport_file_read_method(LogTransport *self, gpointer buf, gsize buflen, LogTransportAuxData *aux)
{
	gint rc;

	do
	{
		rc = read(self->fd, buf, buflen);
	}
	while (rc == -1 && errno == EINTR);

	msg_debug("[CC] BUFFER: ", evt_tag_printf("buf", buf));


	// first and second chars
	gchar *chars = g_strndup(buf, 3);

	msg_debug("[CC] FIRST CHARS: ", evt_tag_printf("chars", chars));

	// TODO @ccaballe avoid memcpy
	if (rc == 0){
		if (chars[0] == '\n' && chars[1] == '\n') {
			msg_debug("[CC] Removing first break line");
			memcpy(buf, buf+2, buflen - sizeof(gchar)- sizeof(gchar));

			msg_debug("[CC] AFTER REMOVING: ", evt_tag_printf("buf", buf));

			return rc;
		}
		// char1 puede ser null, ese caso hay que controlarlo
		// TODO verify NULL ?
		else if (chars[0] == '\n' && chars[1] != '\n') {
			msg_debug("[CC] Processing new line from buffer");

			rc = 1;
		}
		else {
			msg_debug("[CC] Injecting new line in buffer");
			gpointer buf2 = malloc(buflen + sizeof(gchar));
			gchar br = '\n';
			memcpy(buf2, &br, sizeof(br));
			memcpy(buf2 + sizeof(br), buf, buflen);
			memcpy(buf, buf2, buflen + sizeof(gchar));
		}
	}


	return rc;
}

gssize
log_transport_file_read_and_ignore_eof_method(LogTransport *self, gpointer buf, gsize buflen, LogTransportAuxData *aux)
{
	gint rc;

	rc = log_transport_file_read_method(self, buf, buflen, aux);
	if (rc == 0)
	{
		rc = -1;
		errno = EAGAIN;
	}
	return rc;
}

gssize
log_transport_file_write_method(LogTransport *self, const gpointer buf, gsize buflen)
{
	gint rc;

	do
	{
		rc = write(self->fd, buf, buflen);
	}
	while (rc == -1 && errno == EINTR);
	return rc;
}

void
log_transport_file_init_instance(LogTransportFile *self, gint fd)
{
	log_transport_init_instance(&self->super, fd);
	self->super.read = log_transport_file_read_method;
	self->super.write = log_transport_file_write_method;
	self->super.free_fn = log_transport_free_method;
}

LogTransport *
log_transport_file_new(gint fd)
{
	LogTransportFile *self = g_new0(LogTransportFile, 1);

	log_transport_file_init_instance(self, fd);
	return &self->super;
}
