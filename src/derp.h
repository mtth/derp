#ifndef DERP_H_
#define DERP_H_

#define DERP_MAX_MSG_LEN 127
// This (short) length lets us considerably simplify handling sending messages:
// we can encode the length in a single byte and retain negative values for
// error signaling. In the future, this client "class" could be enhanced to
// support longer messages.

typedef struct derp derp_t;

/**
 * Create a new client.
 *
 * A client is a simple structure that allows sending and receiving messages
 * (currently only null-terminated strings), while handling the network I/O
 * when the underlying descriptor is available.
 *
 * Messages are sent via the `derp_send_msg` function, then buffered until they
 * can be written to the socket during a call to `derp_on_writable_fd`.
 * Likewise, `derp_on_readable_fd` will populate a buffer of incoming messages
 * which can then be queried via the `derp_recv_msg` function.
 *
 */
derp_t *derp_new(void);

/**
 * Attempt to read a message.
 *
 * If a message is available, it will be copied into `dst` and the function
 * will return its length (including the null byte if the message is a
 * null-terminated string). This function will typically be called inside a
 * `while` loop. If no messages are available, the function returns `-1`.
 *
 */
int derp_recv_msg(derp_t *p, char *dst);

/**
 * Attempt to send a message.
 *
 * The message will be buffered until it can be written to the socket. The
 * function returns 0 if OK, -1 if the message is too long to be sent, and -2
 * if there is currently not enough space to buffer it.
 *
 */
int derp_send_msg(derp_t *p, const char *src, char len);

/**
 * Load the receiving buffer with any incoming network data.
 *
 * Returns 0 if OK, -1 if the incoming data cannot fit in the internal buffer,
 * something went wrong when reading the socket, or no data was read (typically
 * meaning the socket was closed).
 *
 */
int derp_on_readable_fd(derp_t *p, int fd);

/**
 * Flush the sending buffer to the socket.
 *
 * Returns the number of bytes remaining in the buffer if OK, and -1 if
 * something went wrong while writing to the socket.
 *
 */
int derp_on_writable_fd(derp_t *p, int fd);

/**
 * Free resources associated with the client.
 *
 */
void derp_del(derp_t *p);

#endif
