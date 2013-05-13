/*
 * Copyright (c) 2012 Wave Semiconductor Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software without 
 * specific, written prior permission.  The copyright holders make no 
 * representations about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <stdio.h>
#include <fcntl.h>

// http://www.cs.ualberta.ca/~mburo/orts/doxygen/html/popen2_8c-source.html 

//FUNCTION  DEFINITION:
 
/*  Pre:  The  shell  command  that  should  be  executed  is  passed  as  a  parameter.
 *            Values  of  0  for  p_fd_in  or  p_fd_out  indicates  shell  command  doesn't
 *            use  stdin  or  stdout  respectively,  so  they  should  be  closed.
 *  Post:  Makes  accessible  both  the  standard  input  and  output  of  the  shell  
 *              process  it  creates.
 *              Two  pipes  are  created:  one  provides  standard  input  for  the  shell  
 *              commmand,  and  the  other  for  passing  back  its  standard  output.
 *              The  pipe  file  descriptors  for  the  caller  to  use  are  pointed  to  by  
 *              p_fd_in  and  p_fd_out.
 */      
pid_t popen2(const char *shell_cmd, int *p_fd_in, int *p_fd_out)
{
#if 0
  //CREATING  TWO  PIPES:
  int fds_processInput[2];  //pipe  for  process  input
  int fds_processOutput[2]; //pipe  for  process  output
   
  if(pipe(fds_processInput) != 0) //create  process  input  pipe
  {
    fprintf(stderr, "pipe (process input) failed\n");
    exit(1);
  }
   
  if(pipe2(fds_processOutput, O_NONBLOCK) != 0) //create  process  output  pipe
  {
    fprintf(stderr, "pipe (process output) failed\n");
    exit(1);
  }
   
  //FORKING  A  CHILD  PROCESS:
  pid_t pid;
  if((pid = fork()) < 0)
  {
    fprintf(stderr," fork failed\n");
    exit(2);
  }
   
  //CONNECT  THE  CORRECT  PIPE  ENDS  IN  THE  CHILD:
  if(pid == 0)  //child  process
  {
    //for  process  input  pipe:
    close(fds_processInput[1]);   //close  output
    dup2(fds_processInput[0], 0); //close  fd  0,  fd  0  =  fds_processInput[0]
       
    //for  process  output  pipe:
    close(fds_processOutput[0]);   //close  input
    dup2(fds_processOutput[1], 1); //close  fd  1,  fd  1  =  fds_processOutput[1]
       
    execl("/bin/sh", "sh", "-c", shell_cmd, 0 ); 
    fprintf(stderr, "failed to run shell_cmd\n");
  }
  else  //parent  process
  {
    //for  process  input  pipe:
    close(fds_processInput[0]);   //close  input
       
    //for  process  output  pipe:
    close(fds_processOutput[1]);   //close  output
 
    if(p_fd_in == 0)
      close(fds_processInput[1]);
    else
      *p_fd_in = fds_processInput[1];
       
    if(p_fd_out == 0)
      close(fds_processOutput[0]);
    else
      *p_fd_out = fds_processOutput[0];
 
  }
  return pid; 
#endif
}

