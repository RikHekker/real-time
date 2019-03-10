/*
 * 2IN60, Exercises 1
 */

int main(void) { 
  int i = 0;
  int j = 0;
  int k = 0;
  
  while (i < 40) {
    j = 0;
    while (j < i) {
      k++;
      j++;
    }
    i++;
  }
  
  (void)k;
	
  return (0);
}
