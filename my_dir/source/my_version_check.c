#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* ver_add_2(char* ver,int pos,int val)
{
  int i,j=0;

  for (i=strlen(ver)-1,j=0;i>-1;i--)
  {
	if (ver[i] >= '0' && ver[i] <= '9'){
	  j++;
	}
	if (j-1 == pos){
	  break;
	}
  }

  if (i<0) return NULL;

  int isOver= (ver[i]-'0'&0xff)+val>9?1:0;
  ver[i] = isOver?ver[i]+val-10:ver[i]+val;

  if (isOver) ver_add_2(ver,pos+1,1);
  return ver;
}

static char* ver_add(char* outVer,int num)
{
  int i,k,x;
  i=0;
  while(1)
  {
	x = num%10;
	num = num/10;
	if (x == 0 && num== 0) break;
	ver_add_2(outVer,i,x);
	i++;
  }
}

static void set_txt_title(FILE* fd,const char* gitBranch,const char* baseVer,\
	const char* lastVer,const char* lastCommitId,const char* lastChCommit)
{
  int i,j;
  char buf[1024];

  fseek(fd,0,SEEK_SET);

  if (lastVer == NULL || lastCommitId == NULL || lastChCommit == NULL){
	memset(buf,32,sizeof(buf));
	buf[500]='\n';
	buf[501]='\0';
	fwrite(buf,1,strlen(buf),fd);
	return ;
  }

  strcpy(buf,"lastCommitId:");
  fwrite(buf,1,strlen(buf),fd);
  strcpy(buf,lastCommitId+7);
  fwrite(buf,1,strlen(buf),fd);
  

  if (lastChCommit != NULL){
	strcpy(buf,"last Chinese commit:");
	fwrite(buf,1,strlen(buf),fd);
	fwrite(lastChCommit,1,strlen(lastChCommit),fd);
	fwrite("\n\n\n\n",1,1,fd);
  }

  sprintf(buf,"gitBranch:%s,startVer:%s,latestVer:%s\n",\
		gitBranch,baseVer,lastVer);
  fwrite(buf,1,strlen(buf),fd);

  fwrite("\n\n\n",1,3,fd);
}

static char* read_last_commit(FILE* fd,char* buf)
{
  char readbuf[300];
  int i;
  fseek(fd,-sizeof(readbuf)+1,SEEK_END);
  fread(readbuf,1,sizeof(readbuf),fd);

  for (i=strlen(readbuf)-2;i>-1;i--)
  {
	if (readbuf[i]=='\n') break;
  }
//  printf("lastCommit:%s\n",readbuf+i+1);
  strcpy(buf,readbuf+i+1);
}

static char* read_last_commit_a(const char* logFile,const char* segName,char* outBuf)
{
  char readbuf[1024];
  int i;
  FILE* fd = fopen(logFile,"r");
  char* retBuf =NULL;

  outBuf[0]='\0';

  if (fd == NULL ){
	perror("fopen");
	return NULL;
  }

  while ( !feof(fd) )
  {
    char* pAt=NULL;
    if (fgets(readbuf,sizeof(readbuf),fd) == NULL) continue;
    if ((pAt=strstr(readbuf,segName)) == NULL) continue;
    pAt = pAt + strlen(segName)+1;
    for (i=0;i<strlen(pAt);i++)
	if (pAt[i] == ',' || pAt[i] == '\n'){
	  pAt[i]='\0'; break;
	}
    retBuf=outBuf;
    strcpy(outBuf,pAt);
//    printf("%s\n",pAt);
    break;
  }

  fclose(fd);
  return retBuf;
}

int is_need_update_ver(const char* verFile)
{

//  const char* verFile = "a.txt";

  FILE* fd = popen("git log ","r");
  char buf[1024];
  fgets(buf,sizeof(buf),fd);
  fclose(fd);

  char buf2[sizeof(buf)];
  read_last_commit_a(verFile,"lastCommitId",buf2);
  if (strlen(buf2) < 1){
	return 1;
  }

  if (strstr(buf,buf2)){
	return 0;
  }
  return 1;
}

int update_txt_file_ver( const char* gitBranch, \
	const char* baseVer, const char* writeFile)
{
/*
  if (argc != 4){
	printf("argc err\n");
	printf("gitBranch baseVer writeFile\n");
	return -1;
  }

  const char* gitBranch = argv[1];
  const char* baseVer = argv[2];
  const char* writeFile = argv[3];
  const char* gitBranch = "master";
  const char* baseVer = "1.0.00";
  const char* writeFile = "a.txt";
*/

  FILE* fd = popen("git log","r");
  //FILE* fd = fopen("a.txt","r");
  FILE* fdSave= fopen(writeFile,"wb");
  if (fd == NULL || fdSave == NULL){
	perror("fopen");
	return -1;
  }

  char buf[1024];
  int  i=0;
  char curVer[30];
  char lastVer[30];
  char lastChComment[sizeof(buf)];
  char lastCommitId[sizeof(buf)];

  set_txt_title(fdSave,gitBranch,baseVer,NULL,NULL,NULL);

  int isWrite =0;
  int count =0;
  lastVer[0]='\0';
  lastChComment[0]='\0';
  lastCommitId[0]='\0';
  while( !feof(fd) )
  {
	const char* temp=fgets(buf,sizeof(buf),fd);

	if (!isWrite && feof(fd)){
	  isWrite =1;
	  fclose(fd);
	  fd = popen("git log","r");
	  strcpy(lastVer,baseVer);
	  ver_add(lastVer,count);
	  continue;
	}

	if (temp == NULL) continue;

	if (strlen(lastCommitId) < 1) strcpy(lastCommitId,buf);

	int isFCh=0;
	int chAt =0;
	for (i=0;i<strlen(buf);i++){
	  if ((buf[i]&0xff)>0x7F) {isFCh=1; chAt=i;break;}
	}

	if (!isFCh) continue;

	if (!isWrite) count++;

	if (isWrite){
	  buf[strlen(buf)-1]='\0';

	  strcpy(curVer,baseVer);
	  ver_add(curVer,count--);
	  fwrite(curVer,1,strlen(curVer),fdSave);

	  fwrite("            ",1,10,fdSave);
	  fwrite(buf+chAt,1,strlen(buf+chAt),fdSave);
	  fwrite("\n\n",1,1,fdSave);

	  if (strlen(lastChComment) < 1) strcpy(lastChComment,buf+chAt);
	}
  }


  set_txt_title(fdSave,gitBranch,baseVer,lastVer,lastCommitId,lastChComment);

  fclose(fdSave);
  fclose(fd);
}
#if 0
int main()
{
  is_need_update_ver("a.txt");
  return 0;
}
#endif
