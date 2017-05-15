#include "types.h"
#include "stat.h"
#include "user.h"
#define MAXLEN 20


int checkpasswd(char *user, char *passwd){
  int i, n, c,l;
  char ipasswd[MAXLEN];
  char iuser[MAXLEN];
  char buf[1024];
  int fd;

  l = c = 0;
  fd = open("passwd", 0);
  while((n = read(fd, buf, sizeof(buf))) > 0){
    for(i=0; i<n;) {

      if(l == 0){
      	while(i < n && buf[i] != ':' )iuser[c++] = buf[i++];
      	if(i == n) break;
      	iuser[c] = '\0';
      	i++;
      }
      while(i <n && buf[i] != ':')ipasswd[l++] = buf[i++];
      if(i == n) break;
      ipasswd[l] = '\0';
      c = 0;
      l = 0;

      if(!strcmp(user,iuser) && !strcmp(passwd,ipasswd))
      	return 1;
      while(i <n && buf[i++] != '\n');

    }
  }
  close(fd);
  return 0;

}


int main(int argc, char *argv[]){
  char user[MAXLEN];
  char passwd[MAXLEN];
  int pid,wpid;
  char *shargv[] = { "sh", 0 };



  while(1){
    printf(1, "user login:");
    gets(user,MAXLEN);
    printf(1,"password:");
    gets(passwd,MAXLEN);
    user[strlen(user)-1]='\0';
    passwd[strlen(passwd)-1]='\0';
    if(checkpasswd(user,passwd)){
      pid = fork();
      if(pid < 0){
        printf(1, "init: fork failed\n");
        exit();
      }
      if(pid == 0){
        exec("sh", shargv);
        printf(1, "init: exec sh failed\n");
        exit();
      }
      while((wpid=wait()) >= 0 && wpid != pid)
        printf(1, "zombie!\n");
    }
    else{
      printf(1,"wrong user or password\n\n");
    }


  }




  exit();
}
