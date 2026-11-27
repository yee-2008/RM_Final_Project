#ifndef REMOTE_FRAME_H
#define REMOTE_FRAME_H

#include <stdint.h>

/* Feed bytes from the Bluetooth stream in receive order. A complete, valid
 * line is submitted with the timestamp of its first byte (a delayed LF cannot revive a stale command).
 */
void RemoteFrame_Byte(uint8_t byte, uint32_t received_ms);

/* Discard a partial line and clear the remote link state on disconnect. */
void RemoteFrame_Reset(void);

#endif
