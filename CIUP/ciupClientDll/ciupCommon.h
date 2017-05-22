// Common definitions between CIUP client and server

#pragma once

enum ciupStatus {
	UNKNOWN
	// TODO
};

#pragma pack(1)
typedef struct {
	ciupStatus status = UNKNOWN;
	// TODO
} ciupServerInfo;
#pragma pack()