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
#include "../include/Circular.h"

int16_t *circular_buff;
int32_t writter_index;
pthread_mutex_t mutex;
pthread_mutex_t wuw_mutex;
pthread_cond_t wuw_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t vad_end_cond = PTHREAD_COND_INITIALIZER;
uint16_t max_buf_size = 16000/2; //default size

/**
* Add some data in the global circular buffer.
* Returns the next write index
* Param: Current index to start writing
* Param: Pointer to the data
* Param: Number of elements to write
*/
uint32_t add(uint32_t index, int16_t* data, int32_t size) {
  if (max_buf_size > BUFSIZE) { // Useless case if bufsize > max_buf_size
   if (size + index >= max_buf_size) { //If there is not enough places at the end of the buffer
     unsigned int free_place = max_buf_size - index; //compute the free place in the buffer
     memcpy(circular_buff+index,data,free_place*sizeof(int16_t)); //copy data at the end of the buffer
     memcpy(circular_buff,data+free_place,(size-free_place)*sizeof(int16_t));//copy end of data at the start of the buffer
     return size-free_place; //Return the new writing index
   }
   memcpy(circular_buff+index,data,size*sizeof(int16_t));
   return index+size;
 }
 return 0;
}
