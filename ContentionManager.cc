#include "ContentionManager.hh"

#define MAX_TS UINT_MAX
#define TS_THRESHOLD 10
#define SUCC_ABORTS_MAX 10
#define WAIT_CYCLES_MULTIPLICATOR 8000

bool ContentionManager::should_abort(Transaction* tx, WriteLock wlock) {
	//printf("In CM::should_abort.\n");
    int threadid = tx->threadid();
    //printf("CM::should_abort: threadid = [%d].\n", threadid);
    //printf("CM::should_abort: abort_count = [%d].\n", (int)abort_count[threadid]);
    if (aborted[threadid] == 1)
        return true;

    if (timestamp[threadid] == MAX_TS)
        return true;

    int owner_threadid = wlock & TransactionTid::threadid_mask; 
    if (timestamp[owner_threadid] < timestamp[threadid]) {
        if (aborted[owner_threadid] == 0) {
           return true; 
        } else {
           return false;
        }
    } else {
    	//FIXME: this might abort a new transaction on that thread
        aborted[owner_threadid] = 1;
        return false;
    }
}

void ContentionManager::on_write(Transaction* tx) {
	//printf("In CM::on_write.\n");
    int threadid = tx->threadid();
    //printf("CM::on_write: thread id is [%d].\n", threadid);
    //printf("CM::on_write: abort_count = [%d].\n", (int)abort_count[threadid]);
    write_set_size[threadid] += 1;
    if (ts == MAX_TS && write_set_size[threadid] == TS_THRESHOLD) {
        timestamp[threadid] = fetch_and_add(&ts, uint64_t(1));
    }
}

void ContentionManager::start(Transaction *tx) {
	//printf("In CM::start.\n");
    int threadid = tx->threadid();
    if (tx->is_restarted()) {
        // Do not reset timestamp and abort count
        aborted[threadid] = 0;
        write_set_size[threadid] = 0;
        version[threadid] += 1;
    } else {
        timestamp[threadid] = MAX_TS;
        aborted[threadid] = 0;
        write_set_size[threadid] = 0;
        abort_count[threadid] = 0; 
        version[threadid] += 1;
    }

}

void ContentionManager::on_rollback(Transaction *tx) {
    int threadid = tx->threadid();
    //printf("CM::on_rollback: threadid = [%d].\n", threadid);
    if (abort_count[threadid] < SUCC_ABORTS_MAX)
        ++abort_count[threadid];
    uint64_t cycles_to_wait = rand() % (abort_count[threadid] * WAIT_CYCLES_MULTIPLICATOR);
    //printf("CM::on_rollback: before wait.\n");
    wait_cycles(cycles_to_wait);
    //printf("CM::on_rollback: after wait.\n");
 }

// Defines and initializes the static fields
uint64_t ContentionManager::ts = 0;
uint128_t ContentionManager::aborted[32] = { 0 };
uint128_t ContentionManager::timestamp[32] = { 0 };
uint128_t ContentionManager::write_set_size[32] = { 0 };
uint128_t ContentionManager::abort_count[32] = { 0 };
uint128_t ContentionManager::version[32] = { 0 };