#include <stdio.h>

int main(void){
    char * filename = "data.txt";  
    char buff[256];
    int speed_x=0;
    int speed_y=0;
    int speed_z=0;
    double pitch=0;
    double roll=0;
    double yaw=0;
    FILE *fp=fopen(filename,"r");
    if(fp){
        while (fgets(buff,256,fp)!=NULL)
        {
            speed_x=(int)buff;
            speed_y=(int)buff;
            speed_z=(int)buff;
            pitch=(double)((int)buff);
            roll=(double)((int)buff);
            yaw=(double)((int)buff);
        }
        fclose(fp);
    }
 return 0;   
}