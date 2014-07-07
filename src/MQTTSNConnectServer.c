/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "StackTrace.h"
#include "MQTTSNPacket.h"
#include <string.h>

#define min(a, b) ((a < b) ? 1 : 0)


/**
  * Deserializes the supplied (wire) buffer into connect data structure
  * @param data the connect data structure to be filled out
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param len the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success, 0 is failure
  */
int MQTTSNDeserialize_connect(MQTTSNPacket_connectData* data, unsigned char* buf, int len)
{
	MQTTSNFlags flags;
	unsigned char* curdata = buf;
	unsigned char* enddata = &buf[len];
	int rc = 0;
	int version;
	int mylen = 0;

	FUNC_ENTRY;
	curdata += (rc = MQTTSNPacket_decode(curdata, len, &mylen)); /* read length */
	enddata = buf + mylen;
	if (enddata - curdata < 2)
		goto exit;

	if (readChar(&curdata) != MQTTSN_CONNECT)
		goto exit;

	flags.all = readChar(&curdata);
	data->cleansession = flags.bits.cleanSession;
	data->willFlag = flags.bits.will;

	if ((version = (int)readChar(&curdata)) != 1) /* Protocol version */
		goto exit;

	data->duration = readInt(&curdata);

	if (!readMQTTSNString(&data->clientID, &curdata, enddata))
		goto exit;

	rc = 1;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Serializes the connack packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param connack_rc the integer connack return code to be used 
  * @return serialized length, or error if 0
  */
int MQTTSNSerialize_connack(unsigned char* buf, int buflen, int connack_rc)
{
	int rc = 0;
	unsigned char *ptr = buf;

	FUNC_ENTRY;
	if (buflen < 3)
	{
		rc = MQTTSNPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}

	ptr += MQTTSNPacket_encode(ptr, 4); /* write length */
	writeChar(&ptr, MQTTSN_CONNACK);
	writeChar(&ptr, connack_rc);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Deserializes the supplied (wire) buffer into disconnect data - optional duration
  * @param duration returned integer value of the duration field, -1 if no duration was specified
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param len the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success, 0 is failure
  */
int MQTTSNDeserialize_disconnect(int* duration, unsigned char* buf, int buflen)
{
	unsigned char* curdata = buf;
	unsigned char* enddata = NULL;
	int rc = -1;
	int mylen;

	FUNC_ENTRY;
	curdata += (rc = MQTTSNPacket_decode(curdata, buflen, &mylen)); /* read length */
	enddata = buf + mylen;
	if (enddata - curdata < 1)
		goto exit;

	if (readChar(&curdata) != MQTTSN_DISCONNECT)
		goto exit;

	if (enddata - curdata == 2)
		*duration = readInt(&curdata);
	else if (enddata != curdata)
		goto exit;

	rc = 1;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Serializes a willtopicreq packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @return serialized length, or error if 0
  */
int MQTTSNSerialize_willtopicreq(unsigned char* buf, int buflen)
{
	int rc = 0;
	unsigned char *ptr = buf;

	FUNC_ENTRY;
	if (buflen < 2)
	{
		rc = MQTTSNPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}

	ptr += MQTTSNPacket_encode(ptr, 2); /* write length */
	writeChar(&ptr, MQTTSN_WILLTOPICREQ);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Serializes a willmsgreq packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @return serialized length, or error if 0
  */
int MQTTSNSerialize_willmsgreq(unsigned char* buf, int buflen)
{
	int rc = 0;
	unsigned char *ptr = buf;

	FUNC_ENTRY;
	if (buflen < 2)
	{
		rc = MQTTSNPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}

	ptr += MQTTSNPacket_encode(ptr, 2); /* write length */
	writeChar(&ptr, MQTTSN_WILLMSGREQ);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}



/**
  * Deserializes the supplied (wire) buffer into pingreq data
  * @param clientID the connect data structure to be filled out
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param len the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success, 0 is failure
  */
int MQTTSNDeserialize_pingreq(MQTTString* clientID, unsigned char* buf, int len)
{
	unsigned char* curdata = buf;
	unsigned char* enddata = &buf[len];
	int rc = 0;
	int mylen = 0;

	FUNC_ENTRY;
	curdata += (rc = MQTTSNPacket_decode(curdata, len, &mylen)); /* read length */
	enddata = buf + mylen;
	if (enddata - curdata < 1)
		goto exit;

	if (readChar(&curdata) != MQTTSN_PINGREQ)
		goto exit;

	if (!readMQTTSNString(clientID, &curdata, enddata))
		goto exit;

	rc = 1;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Serializes a pingresp packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @return serialized length, or error if 0
  */
int MQTTSNSerialize_pingresp(unsigned char* buf, int buflen)
{
	int rc = 0;
	unsigned char *ptr = buf;

	FUNC_ENTRY;
	if (buflen < 2)
	{
		rc = MQTTSNPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}

	ptr += MQTTSNPacket_encode(ptr, 2); /* write length */
	writeChar(&ptr, MQTTSN_PINGRESP);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}
