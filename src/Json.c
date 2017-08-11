/*
 ============================================================================
 Name        : Json.c
 Author      : f
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include "Gmalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <parson.h>
#include <string.h>
void JsonMake(char *dat) {
	JSON_Value *root_value;
	JSON_Object *root_object;
	char *serialized_string = NULL;
	// json_object_set_string(root_object, "name", "John Smith");
	//json_object_set_number(root_object, "age", 25);
	//json_object_dotset_string(root_object, "address.city", "Cupertino");
	root_value = json_value_init_object();
	root_object = json_value_get_object(root_value);
	json_object_dotset_value(root_object, "Lock",
			json_parse_string("{\"LockUser\":123456,\"LockPwd\":\"123456\"}"));
	serialized_string = json_serialize_to_string_pretty(root_value);
	memcpy(dat, serialized_string, strlen((char *) serialized_string));
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);

}
char JsonCmpString(char *json, char *jsonCmp, char *out) {
	JSON_Value *root_value;
	JSON_Object *object;
	char error = -1;
	char * value;
	root_value = json_parse_string(json);
	if (root_value != NULL) {
		object = json_value_get_object(root_value);
		if (object != NULL) {
			value = (char *) json_object_dotget_string(object, jsonCmp);
			if (value != 0) {
				memcpy(out, value, strlen(value));
				error = 0;
			}
		}
	}
	json_value_free(root_value);
	return error;
}
typedef struct Validate {
	char User[12];
	char Pwd[8];
} Validate_t, *PValidate;
typedef struct LockCmd {
	char cmd[10];
	char time[10];
//	void send(unsigned char *dat,unsigned short siz);
} LockCmd_t, *PLockCmd;
typedef struct Lock {
	Validate_t validate;
	LockCmd_t lockcmd;
} Lock_t, *PLock;

typedef struct LockRec {
	char User[12];
	unsigned long time;
} LockRec_t, *PLockRec;

char StringTohex(char *dat) {
		char out_dat = 0;
		unsigned short i = 0;
		for (i = 0; i < 2; i++) {
			out_dat = out_dat << 4;
			if ((*(dat + i) >= 'A') && (*(dat + i) <= 'F')) {
				out_dat |= (*(dat + i) - 'A' + 10);
			}
			if ((*(dat + i) >= '0') && (*(dat + i) <= '9')) {
				out_dat |= *(dat + i) - '0';
			}
		}
		return out_dat;
}
void HexToString(char dat, char *out) {
	char buff[] = "0123456789ABCDEFG";
	*(out) = buff[(dat >> 4) & 0x0f];
	*(out + 1) = buff[(dat) & 0x0f];
}
unsigned long StringToLong(char *dat,unsigned short siz)
{
	unsigned long dtime=0;
	char hex;
	if(siz%2==0)
	{
		for(unsigned short  i=0;i<siz;i+=2)
		{
		hex=StringTohex(dat+i);
		dtime=(dtime<<8)|hex;
		}
	}
    return dtime;
}
void LongToString(unsigned long dat,char *out)
{
	unsigned short osiz=0;
	for(unsigned short i=0;i<sizeof(unsigned long);i++)
	{
	HexToString((dat>>(((sizeof(unsigned long)-i-1)*8))&0xff),out+osiz);
	osiz+=2;
	}
	*(out+osiz)=0;
}
LockRec_t LockWriteRec(PLock lock)
{
	LockRec_t rec;
    memcpy(rec.User,lock->validate.User,sizeof(rec.User));
    rec.time=StringToLong(lock->lockcmd.time,strlen(lock->lockcmd.time));
    return rec;
}
char LockHandle(PLock lock, char *Cjson) {
	char error = -1;
	char Ojson[100];
	memset(Ojson, 0, sizeof(Ojson));
	if (!((JsonCmpString(Cjson, "Lock.User", Ojson) == 0)
			&& (memcmp(Ojson, lock->validate.User, strlen(lock->validate.User))
					== 0))) {
		printf("rdat:%s Validate:%s\r\n", Ojson, lock->validate.User);
		printf("User:Error");
		return error;
	}
	memset(Ojson, 0, sizeof(Ojson));
	if (!((JsonCmpString(Cjson, "Lock.Pwd", Ojson) == 0)
			&& (memcmp(Ojson, lock->validate.Pwd, strlen(lock->validate.Pwd))
					== 0))) {
		printf("Pwd:Error");
		return error;
	}
	memset(Ojson, 0, sizeof(Ojson));
	if (JsonCmpString(Cjson, "Lock.Cmd", Ojson) == 0) {
		memcpy(lock->lockcmd.cmd, Ojson, strlen(Ojson));
		printf("Cmd:%s,OK \r\n", Ojson);
		//error = 0;
		//return -1;
	}
	memset(Ojson, 0, sizeof(Ojson));
	if (JsonCmpString(Cjson, "Lock.RecU", Ojson) == 0) {

		printf("Cmd:%ld,OK \r\n", StringToLong(Ojson,strlen(Ojson)));
		error = 0;
			//return -1;
	}

	printf("----OK----\r\n");
	return error;
}
Lock_t Lock;
char stack[700];
int main(void) {
	GmallocInit((void*)stack, sizeof(stack));
	//char  *cmd="{\"Lock\":[{\"SetUser\":\"12345\"},{\"SetPwd\":\"1234565\"}]}";
	char *cmd ="{\"Lock\":{\"User\":\"12345\",\"Pwd\":\"45678\",\"Cmd\":\"Open\",\"RecU\":\"12345678\"}}";
	memcpy(Lock.validate.User, "12345", strlen("12345"));
	memcpy(Lock.validate.Pwd, "45678", strlen("45678"));

	while(1)
	{
	LockHandle(&Lock, cmd);
	printf("Siz:%ld\r\n",GetMallocSiz());
	}
	//char buff[50];
//	LongToString(0x123456,buff);
//    printf("buff:%s,Siz:%d\r\n",buff,strlen(cmd));
////	printf("%ld",StringToLong(buff,strlen(buff)));
	return EXIT_SUCCESS;
}
