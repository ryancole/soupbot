#define BLIZZ 0
#define SPEAKER 1
#define OP 2
#define BRX 3
#define CHAT 4
#define DRTL 5
#define DSHR 6
#define D2 7
#define SC 8
#define SSHR 9
#define BW 10
#define WAR2 11
#define D2X 12
#define SPAWN 13
#define BNET 14
#define SCJ 15
#define JSPAWN 16
#define WAR3 11

char *mystrcat(char *dest, char *src, ...);
int StringCopy(char *dest, char *source, int stopchar);
char *ParseD2Stats(char *stats);
char *ParseW3Stats(char *stats);
void ParseStatstring(char *statstring, char *outbuf);
int GetIconCode(char *prog, int flags);

/* DecodeCDKey
 * This function will take in the CD-Key as a string, and output the three
 * values.  It will also return true or false depending upon whether or not
 * the key is valid for an installation. */

bool DecodeCDKey(const char*, unsigned long *, unsigned long *, unsigned long *);

/* DecodeStarcraftKey
 * Decodes amd checks a Starcraft CD-key */
 
bool DecodeStarcraftKey(char*);

/* DecodeD2Key
 * Decodes and checks a Diablo 2 CD-key */

bool DecodeD2Key(char*);

/* Get_Hex_Value
 * Converts a number into an ASCII hexadecimal digit */

char Get_Hex_Value(unsigned long);

/* Get_Num_Value
 * Converts an ASCII hexadecimal digit into a number */

int Get_Num_Value(char);

/* HashData
 * The original data hashing function by Adron */

void HashData(void* lpSource, int nLength, void* lpResult);

/* getvalues
 * Packet parsing function */

void getvalues(const char *databuf, unsigned long *ping, unsigned long *flags, char *name, char *txt);

/* CheckRevision
 * The original version checking function by YobGuls */

bool CheckRevision2(const char * lpszFileName1, const char * lpszFileName2, const char * lpszFileName3, const char * lpszValueString, unsigned long * lpdwVersion, unsigned long * lpdwChecksum, char * lpExeInfoString, const char * lpszMpqFileName);

/* RealmHash
 * The hashing function for Diablo II/Lord of Destruction realms */

bool RealmHash(char *OutBuf, unsigned long encryptvalue, unsigned long*val1, unsigned long*val2, unsigned long*val3, unsigned long*val4, unsigned long*val5, int subtract);

/* HashPass
 * The original password hashing function for standard logins */

bool HashPass(char *password, int length, unsigned long*val1, unsigned long*val2, unsigned long*val3, unsigned long*val4, unsigned long*val5, int nothing);

/* CreateAccount
 * Function used for creating accounts */

bool CreateAccount(char *OutBuf, char *password, int nothing1, int nothing2, int nothing3, int nothing4, int nothing5, int nothing6);

/* HashCDKey2
 * The CD-Key hashing function for the version update in 2002 */

bool HashCDKey2(char *OutBuf, unsigned long sessionkey, unsigned long prodid, unsigned long val1, unsigned long val2, unsigned long seed, int nothing1, int nothing2);

/* HashCDKey
 * The original CD-Key hashing function for standard logins */

bool HashCDKey(unsigned long*key, unsigned long*seed, unsigned long*prodid, unsigned long*val1, unsigned long*val2, int nothing1, int nothing2, int nothing3);

