/**
 * Copyright (C) 2018 Linagora
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "../include/Audio.h"


// Global variables
extern uint16_t max_buf_size; //maximum buffer size
int16_t *circular_buff; //pointer to the circular buffer
extern int32_t writter_index; //index to write in the circular buffer
extern pthread_mutex_t mutex; //mutex to protect shared data
extern pthread_cond_t wuw_cond; //condition to unlock record
extern pthread_cond_t vad_end_cond; //condition to lock record
FILE *f; //file or pipe to save data
FILE *meeting_file;
char* channels; //number of channels
char* rate; //sampling rate
extern char* ip; //local broker ip
extern char* port; //local broker port

void create_pipe(char* path) {
  mkfifo(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
}

int main(int argc, char** argv) {
  if (argc<8) {
    printf("Usage: PipeFile Rate Channels Circular_Buffer_Time Broker_ip Broker_port Sub_Topic Sub_Keyword_Start Sub_Keyword_Stop Pub_Topic Pub_Keyword\n");
    printf("PipeFile: Name of the named pipe or the file to save data\n");
    printf("Rate: audio sampling rate\n");
    printf("Channels: Number of channels to be used, 1 for mono, 2 for stereo etc...\n");
    printf("Circular_Buffer_Time: Number of seconds the circular_buff should save data\n");
    printf("Broker_ip: The IP address of the MQTT local broker, use localhost by default\n");
    printf("Broker_port: The port of the MQTT local broker, use 1883 by default\n");
    printf("Meeting_File: Name of the file to save data in meeting mode\n");
    return 1;
  }
  circular_buff = malloc(sizeof(int16_t)* max_buf_size); //Allocating circular buffer
  MQTTClient client;
  channels = argv[3];
  rate = argv[2];
  ip = argv[5];
  port = argv[6];
  max_buf_size = strtof(argv[4],NULL)*strtol(rate,NULL,10)*strtol(channels,NULL,10);
  subscribe(&client);
  if (argc == 9 && strcmp(argv[8],"pipe") == 0) {
    create_pipe(argv[1]);
  }
  pthread_t recorder; // Creating thread
  mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER; // Create the mutex to protect shared variables
  pthread_mutex_t wuw_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t vad_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  f = fopen(argv[1],"w+b"); // Open the binary file
  if (f == NULL) {
    printf("Failed to open %s !\n",argv[1]);
    return 1;
  }
  /*char* meeting_file_name = malloc(sizeof(char)*255);
  memset(meeting_file_name,'\0',255);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  char str_time[12];
  sprintf(str_time, "%ld", tv.tv_sec);
  strcat(meeting_file_name,argv[7]);
  strcat(meeting_file_name,str_time);
  strcat(meeting_file_name,".raw");
  meeting_file = fopen(meeting_file_name,"a+b"); // Open the binary file
  if (meeting_file == NULL) {
    printf("Failed to open %s !\n",meeting_file_name);
    return 1;
  }*/
  if(pthread_create(&recorder, NULL, &record, (void*) NULL) == -1) { // Create the thread to record data
    printf("Thread error\n");
    return 1;
  }
  for(;;) {
    pthread_cond_wait(&wuw_cond, &wuw_mutex);
    f = fopen(argv[1],"w+b"); // Open the binary file to delete last file
    if (f == NULL) {
      printf("Failed to open%s !\n",argv[1]);
      return 1;
    }
    pthread_mutex_lock(&mutex);
    if (max_buf_size > BUFSIZE) {
      fwrite(circular_buff+writter_index,sizeof(int16_t),max_buf_size-writter_index,f); // Write the circular buffer in the pipe
      fwrite(circular_buff,sizeof(int16_t),writter_index,f);
    }
    pthread_mutex_unlock(&mutex);
    printf("Start command record\n");
    pthread_cond_wait(&vad_end_cond, &vad_mutex); //Wait for the vader module to send stop message
    printf("End command record\n");
    fclose(f);
  }
  fclose(meeting_file);
  pthread_join(recorder, NULL); //Wait fot the thread to finish
  free(circular_buff);
  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);
  return 0;
}
