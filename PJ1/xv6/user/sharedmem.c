#include "types.h"
#include "stat.h"
#include "user.h"

#define USERTOP 0xA0000
#define PGSIZE 4096

void
testPassed(void)
{
  printf(1, "....Passed\n");
}

void
testFailed(void)
{
  printf(1, "....FAILED\n");
}

void expectedVersusActualNumeric(char* name, int expected, int actual)
{
  printf(1, "      %s expected: %d, Actual: %d\n", name, expected, actual);
}

void
whenRequestingSharedMemory_ValidAddressIsReturned(void)
{
  printf(1, "Test: whenRequestingSharedMemory_ValidAddressIsReturned...");
  char* sharedPage = shmem_access(0);
  char* highestPage =       (char*)(USERTOP - PGSIZE);
  char* secondHighestPage = (char*)(USERTOP - 2*PGSIZE);
  char* thirdHighestPage =  (char*)(USERTOP - 3*PGSIZE);
  char* fourthHighestPage = (char*)(USERTOP - 4*PGSIZE);

  if(sharedPage == highestPage ||
     sharedPage == secondHighestPage ||
     sharedPage == thirdHighestPage ||
     sharedPage == fourthHighestPage) {
    testPassed();
  } else {
    testFailed();
  }
}

void
afterRequestingSharedMemory_countReturns1()
{
  printf(1, "Test: afterRequestingSharedMemory_countReturns1...");
  int* sharedPage = shmem_access(1);
  int count = shmem_count(1);
  sharedPage[0] = 107;
  if(count == 1) {
    testPassed();
  } else {
    testFailed();
    expectedVersusActualNumeric("'count'", 1, count);
  }

  // silence the error about unused variable
  sharedPage = sharedPage + 0;
}

void
whenSharingAPage_ParentSeesChangesMadeByChild()
{
  printf(1, "Test: whenSharingAPage_ParentSeesChangesMadeByChild...\n");
  char* sharedPage = shmem_access(1);
  char* sharedPage2 = shmem_access(0);
  sharedPage[0] = 107;

  int pid = fork();
  if(pid == 0){
    // in child
    char* childsSharedPage = shmem_access(1);
	char* childsSharedPage2 = shmem_access(0);
    childsSharedPage[0] = childsSharedPage[0] + 5;
	childsSharedPage2[0] = 10;
	int cpid = fork();
	if (cpid == 0) {
	// in grandchild
		char* childChildSharedPage = shmem_access(1);
		childChildSharedPage[0] -= 209;
		exit();
	} else {
	  wait();
	}
    exit();
  } else {
    // in parent
    wait(); // wait for child to terminate
    if(sharedPage[0] == -97 && sharedPage2[0] == 10){
      testPassed();
    } else {
      testFailed();
      expectedVersusActualNumeric("'sharedPage[0]'", -97, sharedPage[0]);
	  expectedVersusActualNumeric("'sharedPage2[0]'",10, sharedPage2[0]);
    }
  }
}

void
whenProcessExits_SharedPageIsFreed()
{
  printf(1, "Test: whenProcessExits_SharedPageIsFreed...");
  int pid = fork();

  if(pid == 0){
    // in child
    char* sharedPage = shmem_access(2);
    sharedPage[0] = 42;
    exit();
  } else {
    // in parent
    wait();
	int count = shmem_count(2);
    char* parentsSharedPage = shmem_access(2);
    if(parentsSharedPage[0] != 42 && count == 0){
      testPassed();
    } else {
      // should be garbage value after being freed, but it's still 42
      testFailed();
      expectedVersusActualNumeric("'parentsSharedPage[0]'", 1, parentsSharedPage[0]);
	  expectedVersusActualNumeric("'count'", 0, count);
    }
  }
}

void
whenSharingAPageBetween2Processes_countReturns2()
{
  printf(1, "Test: whenSharingAPageBetween2Processes_countReturns2...");

  char* sharedPage = shmem_access(0);
  sharedPage = sharedPage + 0;  // silence unused variable error

  int pid = fork();

  if(pid == 0){
    // in child
    char* childsSharedPage = shmem_access(0);
    childsSharedPage = childsSharedPage + 0;  // silence unused variable error

    int count = shmem_count(0);
    if(count != 2){
      testFailed();
      expectedVersusActualNumeric("'count'", 2, count);
    }

    exit();
  } else{
    // in parent
    wait(); // wait for child to exit
    int parentsCount = shmem_count(0);
    if(parentsCount != 1){
      testFailed();
      expectedVersusActualNumeric("'parentsCount'", 1, parentsCount);
    }
  }

  testPassed();
}

void
whenProcessExists_countReturns0()
{
  printf(1, "Test: whenProcessExists_countReturns0...");

  int pid = fork();

  if(pid == 0){
    // in child
    char* sharedPage = shmem_access(0);
    sharedPage = sharedPage + 0;  // silence unused variable error
    exit();
  } else {
    // in parent
    wait();
    int count = shmem_count(0);

    if(count != 0){
      testFailed();
      expectedVersusActualNumeric("'count'", 0, count);
    } else {
      testPassed();
    }

  }
}

void
beforeRequestingSharedMemory_countReturns0()
{
  printf(1, "Test: beforeRequestingSharedMemory_countReturns0...");

  int count = shmem_count(0);

  if(count != 0){
    testFailed();
    expectedVersusActualNumeric("'count'", 0, count);
  } else {
    testPassed();
  }
}

void
checkSharedMemoryAddress() {
  printf(1, "Test: checkSharedMemoryAddress...");
  char* highestPage =       (char*)(USERTOP - PGSIZE);
  char* secondHighestPage = (char*)(USERTOP - 2*PGSIZE);
  char* thirdHighestPage =  (char*)(USERTOP - 3*PGSIZE);
  char* fourthHighestPage = (char*)(USERTOP - 4*PGSIZE);

  char* hPage = shmem_access(0);
  char* sPage = shmem_access(1);
  char* tPage = shmem_access(2);
  char* fPage = shmem_access(3);

  if (hPage == highestPage &&
		sPage == secondHighestPage &&
		tPage == thirdHighestPage &&
		fPage == fourthHighestPage) {
	testPassed();
  } else {
	expectedVersusActualNumeric("'highestPage'", (int)highestPage, (int)hPage);
	expectedVersusActualNumeric("'secondHighestPage'", (int)secondHighestPage, (int)sPage);
	expectedVersusActualNumeric("'thirdHighestPage'", (int)thirdHighestPage, (int)tPage);
	expectedVersusActualNumeric("'fourthHighestPage'", (int)fourthHighestPage, (int)fPage);
	testFailed();
  }
}

void
checkFaultPageNumber() {
  printf(1, "Test: checkFaultPageNumber...");
  char* page1 = shmem_access(100);
  char* page2 = shmem_access(-23);

  if (page1 == NULL && page2 == NULL) {
	testPassed();
  } else {
	expectedVersusActualNumeric("'page1'", 0, (int)page1);
	expectedVersusActualNumeric("'page2'", 0, (int)page2);
	testFailed();
  }
}

void
checkSameVirtualAddressForOneSharedMemory() {
  printf(1, "Test: checkSameVirtualAddressForOneSharedMemory...");
  int* page1, *page2, *page3;
  int pid;

  page1 = shmem_access(2);
  *page1 = 255;
  pid = fork();
  if (pid == 0) {
	page2 = shmem_access(2);
	if (page2 != page1 || *page2 != 255) {
	  expectedVersusActualNumeric("'page2'", (int)page1, (int)page2);
	  expectedVersusActualNumeric("'*page2'", 255, *page2);
	  testFailed();
	}
	*page2 = -127;
	exit();
  }
  wait();
  page3 = shmem_access(2);
  if (page3 != page1 || *page3 != -127) {
    expectedVersusActualNumeric("'page3'", (int)page1, (int)page3);
    expectedVersusActualNumeric("'*page3'", -127, *page3);
	testFailed();
  }
  testPassed();
  exit();
}

void
checkSharedMemoryCount() {
  printf(1, "Test: checkSharedMemoryCount...");
  int pid, cpid;
  int count1, count2;

  shmem_access(1);
  shmem_access(3);
  pid = fork();
  if (pid == 0) {
	// in child
	shmem_access(1);
	shmem_access(3);
	count1 = shmem_count(1);
	if (count1 != 2) {
	  expectedVersusActualNumeric("'1 page number 1'", 2, count1);
	  testFailed();
	}
	cpid = fork();
	if (cpid == 0) {
	  // in grandchild
	  shmem_access(3);
	  count2 = shmem_count(3);
	  if (count2 != 3) {
		expectedVersusActualNumeric("'2 page number 3'", 3, count2);
		testFailed();
	  }
	  exit();
	}
	wait();
	count2 = shmem_count(3);
	if (count2 != 2) {
	  expectedVersusActualNumeric("'3 page number 3'", 2, count2);
	  testFailed();
	}
	exit();
  }
  wait();
  shmem_access(1);
  shmem_access(3);
  count1 = shmem_count(1);
  count2 = shmem_count(3);
  if (count1 != 1 || count2 != 1) {
	expectedVersusActualNumeric("'4 page number 1'", 1, count1);
	expectedVersusActualNumeric("'5 page number 3'", 3, count2);
	testFailed();
  }
  testPassed();
  exit();
}

void
checkSharedMemoryCountFault() {
  printf(1, "Test: checkSharedMemoryCountFault...");
  int count1 = shmem_count(-1);
  int count2 = shmem_count(1000);

  if (count1 == -1 && count2 == -1) {
	testPassed();
  } else {
	expectedVersusActualNumeric("'count1'", -1, count1);
	expectedVersusActualNumeric("'count2'", -1, count2);
	testFailed();
  }
  exit();
}





int
main(void)
{
  int pid;

  // we fork then run each test in a child process to keep the main process
  // free of any shared memory
  pid = fork();
  if(pid == 0){
 whenRequestingSharedMemory_ValidAddressIsReturned();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    afterRequestingSharedMemory_countReturns1();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenSharingAPage_ParentSeesChangesMadeByChild();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenSharingAPageBetween2Processes_countReturns2();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenProcessExits_SharedPageIsFreed();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenProcessExists_countReturns0();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    beforeRequestingSharedMemory_countReturns0();
    exit();
  }
  wait();

  pid = fork();
  if (pid == 0) {
	checkSharedMemoryAddress();
	exit();
  }
  wait();

  pid = fork();
  if (pid == 0) {
	checkFaultPageNumber();
	exit();
  }
  wait();

  pid = fork();
  if (pid == 0) {
	checkSameVirtualAddressForOneSharedMemory();
	exit();
  }
  wait();

  pid = fork();
  if (pid == 0) {
	checkSharedMemoryCount();
	exit();
  }
  wait();

  pid = fork();
  if (pid == 0) {
	checkSharedMemoryCountFault();
	exit();
  }
  wait();

  exit();
}
