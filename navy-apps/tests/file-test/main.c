#include <stdio.h>
#include <assert.h>

#include <unistd.h>
int main() {
	//FILE *fp0 = fopen("/share/music/rhythm/La.ogg", "r+");
	//assert(0);
	FILE *fp = fopen("/share/files/num", "r+");	
  //write(1, "Hello World!\n", 13);
	assert(fp);
  fseek(fp, 0, SEEK_END);
  //write(1, "Hello World!\n", 13);
  long size = ftell(fp);
  //assert(size == 5000);
  //write(1, "Hello World!\n", 13);
  fseek(fp, 500 * 5, SEEK_SET);
	int i, n;
  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }

  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1 + 1000);
  }

  fclose(fp);

  printf("PASS!!!\n");

  return 0;
}
