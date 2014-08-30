#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char* getLastestVer(const char* inFile,char* ver)
{
  FILE* fd =fopen(inFile,"r");
  if (fd == NULL){
        perror("fopen");
        return NULL;
  }

  char buf[100];
  char lastLine[100];
  fseek(fd,-100,SEEK_END);
  int isFound =0;
  int i,j;
  while( !feof(fd) )
  {
        if (fgets(buf,sizeof(buf),fd) == NULL) continue;

        if (strlen(buf) <5) continue;

	isFound =1;
        strcpy(lastLine,buf);
  }
  fclose(fd);

  if (!isFound){
	return NULL;
  }

  for (i=0;i<strlen(lastLine);i++)
  {
        if (lastLine[i]>='0' && lastLine[i]<='9'){
          for (j=0;j<strlen(lastLine+i);j++)
          {
                if ( !(lastLine[i+j]>='0'&& lastLine[i+j]<='9') \
                   &&   lastLine[i+j] != '.'){
                  ver[j]='\0';
                  break;
                }else{
                  ver[j]=lastLine[i+j];
		}
          }
          break;
        }
  }

  return ver;
}

static char* getCurGitVer(char* ver)
{
  char* result = "";
  int i,isFind;
  FILE *fpRead;
  const char* command="git branch";

  fpRead = popen(command, "r");
  char buf[2048];
  memset(buf,'\0',sizeof(buf));

  isFind = 0;
  while(fgets(buf,sizeof(buf)-1,fpRead)!=NULL)
  {
	if (buf[0] == '*'){
	  strcpy(ver,buf+2);
	  ver[strlen(ver)-1]='\0';
	  isFind = 1;
	  break;
	}
  }

  if(fpRead!=NULL) pclose(fpRead);

  if (isFind) return ver;

  return NULL;
}

static char* kernelModifyLine(char* buf,const char* ver)
{
  int i=0;
  for (i=0;i<strlen(buf);i++)
  {
	if (!strncmp(buf+i,"kernel:?",8)){
	  char temp[1024];
	  strcpy(temp,buf+i+8);
	  buf[i+7]='\0';
	  strcat(buf,ver);
	  strcat(buf,temp);
	  break;
	}
  }

  if (i<strlen(buf)) return buf;

  return NULL;
}

static int setVersion(const char* ver)
{
  FILE* fd = fopen("fs/proc/version.sample.c","rb");
  FILE* fdOut = fopen("fs/proc/version.c","wb");
//  FILE* fdOut = fopen("/home/abc/Desktop/one.c","wb");

  if (fd == NULL || fdOut == NULL){
	perror("fopen");
	return -1;
  }


  char buf[1024];
  int i,j;
  while( !feof(fd) )
  {
	if (fgets(buf,sizeof(buf),fd) == NULL) continue;

	kernelModifyLine(buf,ver);

	fwrite(buf,1,strlen(buf),fdOut);
  }

  fclose(fd);
  fclose(fdOut);
  return 0;
}

int main()
{
  char gitVer[20];
  if (getCurGitVer(gitVer) == NULL){
	printf("getCurGitVer failed!\n");
	return -1;
  }

  char openVerName[20];
  sprintf(openVerName,"%s_version.txt",gitVer);

  char lastestVer[20];
  if (getLastestVer(openVerName,lastestVer) == NULL){
	printf("getLastestVer failed!\n");
	return -1;
  }
  printf("lastestVer:%s\n",lastestVer);
  
  char kernelVer[30];
  sprintf(kernelVer,"%s %s",gitVer,lastestVer);

  if (setVersion(kernelVer)){
	printf("setVersion failed!\n");
  }

  return 0;
}
