#ifndef __MQTT_H
#define __MQTT_H

#include <sys/types.h>
#include <string.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')
#define BIT_SELECT(N) (~((unsigned)-1 << N))

void _PrintM(const char* const format, ...);
void print_binary(const char* buffer, const int size) {
  for (int i = size - 1;i >= 0;--i)
    _PrintM(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buffer[i]));
}

#define FORMED_LENGTH(str) strlen(str) + 2

#define MAX_ID 256
#define MAX_TOPIC_LEN 256

extern int packet_id;

int mqtt_packet_id(int* packet) {
  return *packet = (*packet + 1)%MAX_ID;
}

struct mqtt_client_t {
  char* client_name;
  char* username;
  char* password;
  int keepalive;
  int is_ssl;
};

ssize_t mqtt_write(struct mqtt_client_t* const client, const unsigned char* const buffer, const size_t size);

ssize_t mqtt_read(struct mqtt_client_t* const client, unsigned char* const buffer, const size_t size);

void encode_fixed_header(unsigned char** const pointer, const char message_type, const char dup_flag, const char qos_level, const char retain) {
  *((*pointer)++) = 
    (message_type & BIT_SELECT(4)) << 4 | // xxxx0000
    (dup_flag & BIT_SELECT(1)) << 3 |     // 0000x000
    (qos_level & BIT_SELECT(2)) << 1 |    // 00000xx0
    (retain & BIT_SELECT(1));             // 0000000x
}

void decode_fixed_header(unsigned char** const pointer, char* const message_type, char* const dup_flag, char* const qos_level, char* const retain) {
  unsigned char src = *((*pointer)++);
  *message_type = (src >> 4) & BIT_SELECT(4); // xxxx0000
  *dup_flag = (src >> 3) & BIT_SELECT(1);     // 0000x000
  *qos_level = (src >> 1) & BIT_SELECT(2);    // 00000xx0
  *retain = (src) & BIT_SELECT(1);            // 0000000x
}

void encode_byte(unsigned char** const pointer, const int value) {
  *((*pointer)++) = value;
}

void decode_byte(unsigned char** const pointer, int* const value) {
  *value = *((*pointer)++);
}

void encode_formed_byte(unsigned char** const pointer, const int value) {
  *((*pointer)++) = 0;     // MSB
  *((*pointer)++) = value; // LSB
}

void decode_formed_byte(unsigned char** const pointer, int* const value) {
  ++(*pointer);             // MSB
  *value = *((*pointer)++); // LSB
}

void encode_formed_string(unsigned char** const pointer, const char* const value) {
  const int len = strlen(value);
  encode_formed_byte(pointer, len);
  memcpy(*pointer, value, len);
  *pointer += len;
}

void decode_formed_string(unsigned char** const pointer, char* const value, const int len) {
  memcpy(value, *pointer, len);
  value[len] = 0;
  *pointer += len;
}

int encode_remaining_len(unsigned char** const pointer, size_t size) {
  int i = 0;
  do {
    if (i == 4)
      return 1;
    char digit = size%128;
    size /= 128;
    if (size > 0)
      digit = digit | 0x80;
    *((*pointer)++) = digit;
    ++i;
  } while (size > 0);
  return i;
}

void decode_remaining_len(struct mqtt_client_t* const client, unsigned char** const pointer, int* const length) {
  size_t size = 0;
  size_t multi = 1;
  int i = 0;
  char digit = 0;
  do {
    mqtt_read(client, *pointer, 1);
    digit = **pointer;
    size += (digit & 127)*multi;
    multi *= 128;
    ++i;
    (*pointer)++;
  } while ((digit & 128) != 0);
  *length = (int)size;
}

void encode_connect_flags(unsigned char** const pointer, const int username, const int password, const int retain, const int qos, const int will, const int clean_session) {
  *((*pointer)++) = 
    (username & BIT_SELECT(1)) << 7 |
    (password & BIT_SELECT(1)) << 6 |
    (retain & BIT_SELECT(1)) << 5 |
    (qos & BIT_SELECT(2)) << 3 |
    (will & BIT_SELECT(1)) << 2 |
    (clean_session & BIT_SELECT(1)) << 1;
}

int mqtt_connect(struct mqtt_client_t* const client, const char* const topic, const char* const message) {
  unsigned char* pointer = NULL;
  unsigned char connect_header[4];
  pointer = connect_header;
  encode_fixed_header(&pointer, 1, 0, 0, 0);

  const size_t protocol_name_len = FORMED_LENGTH("MQIsdp");
  unsigned char var_header[protocol_name_len + 1 + 1 + 2];
  pointer = var_header;
  encode_formed_string(&pointer, "MQIsdp");
  encode_byte(&pointer, 3);
  encode_connect_flags(&pointer, client->username != NULL, client->password != NULL, 0, 1, (topic) && (message), 0);
  encode_formed_byte(&pointer, client->keepalive);

  size_t payload_size = 0;
  payload_size += FORMED_LENGTH(client->client_name);
  if (topic)
    payload_size += FORMED_LENGTH(topic);
  if (message)
    payload_size += FORMED_LENGTH(message);
  if (client->username)
    payload_size += FORMED_LENGTH(client->username);
  if (client->password)
    payload_size += FORMED_LENGTH(client->password);

  unsigned char payload[payload_size];
  pointer = payload;
  encode_formed_string(&pointer, client->client_name);
  if (topic)
    encode_formed_string(&pointer, topic);
  if (message)
    encode_formed_string(&pointer, message);
  if (client->username)
    encode_formed_string(&pointer, client->username);
  if (client->password)
    encode_formed_string(&pointer, client->password);

  pointer = connect_header + 1;
  int length_size = encode_remaining_len(&pointer, protocol_name_len + 4 + payload_size);

  mqtt_write(client, connect_header, 1 + length_size);
  mqtt_write(client, var_header, protocol_name_len + 4);
  mqtt_write(client, payload, payload_size);

  unsigned char ret;
  mqtt_read(client, &ret, 1);
  pointer = &ret;
  char message_type, dup_flag, qos_level, retain;
  decode_fixed_header(&pointer, &message_type, &dup_flag, &qos_level, &retain);
  unsigned char length_buffer[4];
  int remaining_len = 0;
  pointer = length_buffer;
  decode_remaining_len(client, &pointer, &remaining_len);
  unsigned char ack[remaining_len];
  pointer = ack;
  mqtt_read(client, ack, remaining_len);
  int value;
  decode_formed_byte(&pointer, &value);
  return value;
}

int mqtt_publish(struct mqtt_client_t* client, const char* topic, const char* message, const int qos) {
  if (topic == NULL)
    return 1;

  unsigned char* pointer = NULL;
  unsigned char connect_header[4];
  pointer = connect_header;
  encode_fixed_header(&pointer, 3, 0, qos, 0);

  const size_t var_header_size = 2 + FORMED_LENGTH(topic);
  unsigned char var_header[2 + MAX_TOPIC_LEN];
  pointer = var_header;
  encode_formed_string(&pointer, topic);
  encode_formed_byte(&pointer, mqtt_packet_id(&packet_id));

  pointer = connect_header + 1;
  const int payload_size = strlen(message);
  const int length_size = encode_remaining_len(&pointer, var_header_size + payload_size);

  mqtt_write(client, connect_header, 1 + length_size);
  mqtt_write(client, var_header, var_header_size);
  mqtt_write(client, (unsigned char*)message, payload_size);

  return 0;
}

int mqtt_subscribe_ack(struct mqtt_client_t* client) {
  unsigned char* pointer = NULL;
  unsigned char fixed_header = 0;
  int ret = mqtt_read(client, &fixed_header, 1);
  if (ret == 0) {
    return 1;
  }
  char message_type, dup_flag, qos_level, retain = 0;
  decode_fixed_header(&pointer, &message_type, &dup_flag, &qos_level, &retain);
  unsigned char length_buffer[4] = {0};
  int remaining_len = 0;
  pointer = length_buffer;
  decode_remaining_len(client, &pointer, &remaining_len);
  unsigned char payload[remaining_len];
  ret = mqtt_read(client, payload, remaining_len);
  if (ret == 0) {
    return 1;
  }
  pointer = payload;
  int topic_len = 0;
  decode_formed_byte(&pointer, &topic_len);
  char topic[topic_len + 1];
  memcpy(topic, pointer, topic_len);
  topic[topic_len] = 0;
  int packet_id = 0;
  pointer += topic_len;
  decode_formed_byte(&pointer, &packet_id);
  char data[remaining_len - 4 - topic_len + 1];
  memcpy(data, pointer, remaining_len - 4 - topic_len);
  data[remaining_len - 4 - topic_len] = 0;
  _PrintM("Packet: %d, Topic: %s, Data: %s\n", packet_id, topic, data);
  return 0;
}

#endif
