void manysubmit()
{
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
  
  for(int i=100;i<1000;i++)
    {
      gSystem->Exec(Form("qsub -v counter=%i manysub.qsub",i));
      sleep(1);
    }
}
