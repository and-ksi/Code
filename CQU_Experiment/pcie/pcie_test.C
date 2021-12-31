

typedef struct Head {
	char Board_Type[8];
	char Board_Addr[8];
	char FType[2];
	char Error[14];
};
typedef struct Frame {
	char ID[8];
	char Error[6];
	char FType[2];
	char Lenth[16];
	char Timestamp_H[32];
	char Timestamp_L[32];
};
