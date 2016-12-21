#include "wtmyfile.h"
int write_to_my_file(int val, char filename[100]) { 
int fd; 
char buffer[20];
sprintf(buffer,"%d", val); 
if ((fd=open(filename, O_WRONLY| O_CREAT| O_APPEND))==-1) 
{   
perror("Cannot open file"); 
}
else 
{
ssize_t n;
strcat(buffer,   
write(fd,buffer,strlen(buffer));
close(fd);
}
return 0; 
}

