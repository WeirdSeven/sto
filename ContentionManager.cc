#include "ContentionManager.hh"
#include <fstream>

#define MAX_TS UINT_MAX
#define TS_THRESHOLD 10
#define SUCC_ABORTS_MAX 10
#define WAIT_CYCLES_MULTIPLICATOR 8000

bool ContentionManager::should_abort(Transaction* tx, WriteLock wlock) {	
    int threadid = tx->threadid();
    //std::ofstream outfile;
    //outfile.open(std::to_string(threadid), std::ios_base::app);
    //outfile << "In CM::should_abort" << std::endl;
    //outfile << "CM::should_abort threadid = [" << threadid << "]" << std::endl;
    //outfile << "CM::should_abort abort_count = [" << (int)abort_count[threadid] << "]" << std::endl;
    //outfile << "Print write lock" << std::endl;
    //outfile << wlock << std::endl;
    int owner_threadid = wlock & TransactionTid::threadid_mask; 
    //outfile << "CM::should_abort owner_threadid = [" << owner_threadid << "]" << std::endl;
    if (aborted[threadid] == 1){
    	//outfile << "CM::should_abort already aborted." << std::endl;
    	//outfile.close();
        return true;
    }

    if (timestamp[threadid] == MAX_TS) {
    	//outfile << "CM:should_abort MAX_TS" << std::endl;
    	//outfile.close();
        return true;
    }


    if (timestamp[owner_threadid] < timestamp[threadid]) {
        if (aborted[owner_threadid] == 0) {
           //outfile.close();
           return true; 
        } else {
        	//outfile.close();
           return false;
        }
    } else {
    	//FIXME: this might abort a new transaction on that thread
        aborted[owner_threadid] = 1;
        //outfile.close();
        return false;
    }

}

void ContentionManager::on_write(Transaction* tx) {
    int threadid = tx->threadid();
    //std::ofstream outfile;
    //outfile.open(std::to_string(threadid), std::ios_base::app);
    //outfile << "In CM::on_write" << std::endl;
    //outfile << "CM::on_write threadid = [" << threadid << "]" << std::endl;
    //outfile << "CM::on_write abort_count = [" << (int)abort_count[threadid] << "]" << std::endl;
    write_set_size[threadid] += 1;
    if (ts == MAX_TS && write_set_size[threadid] == TS_THRESHOLD) {
        timestamp[threadid] = fetch_and_add(&ts, uint64_t(1));
    }
    //outfile.close();
}

void ContentionManager::start(Transaction *tx) {	
    int threadid = tx->threadid();
    //std::ofstream outfile;
    //outfile.open(std::to_string(threadid), std::ios_base::app);
    //outfile << "In CM::on_start" << std::endl;
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
    //outfile.close();
}

void ContentionManager::on_rollback(Transaction *tx) {
    int threadid = tx->threadid();
    //std::ofstream outfile;
    //outfile.open(std::to_string(threadid), std::ios_base::app);
    //outfile << "CM::on_rollback threadid = [" << threadid << "]" << std::endl;
    if (abort_count[threadid] < SUCC_ABORTS_MAX)
        ++abort_count[threadid];
    uint64_t cycles_to_wait = rand() % (abort_count[threadid] * WAIT_CYCLES_MULTIPLICATOR);
    wait_cycles(cycles_to_wait);
    //outfile.close();
 }

// Defines and initializes the static fields
uint64_t ContentionManager::ts = 0;
uint128_t ContentionManager::aborted[32] = { 0 };
uint128_t ContentionManager::timestamp[32] = { 0 };
uint128_t ContentionManager::write_set_size[32] = { 0 };
uint128_t ContentionManager::abort_count[32] = { 0 };
uint128_t ContentionManager::version[32] = { 0 };