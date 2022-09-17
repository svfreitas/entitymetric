// g++ -pthread  -p hashmap.cpp -lcurl
#include <iostream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <unistd.h>
#include <curl/curl.h>
#include <sstream>

std::mutex m;
std::string s = "ABCBADCA";
std::atomic<bool> thread_done(true);
std::thread t1;
std::unordered_map<std::string , int> equipment_counter;


int sendMetrics(std::string *data)
{
 
  CURLcode ret;
  CURL *hnd;
  struct curl_slist *slist1 = NULL;
 
  slist1 = curl_slist_append(slist1, "Content-Type: text/plain"); 
  slist1 = curl_slist_append(slist1, "Content-Type: text/plain");
  
  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_URL, "https://webhook.site/7a809468-fba9-430d-96ef-e632850aa12a");
  curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, data->c_str());
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.38.0");
  curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
  curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
  curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);

  ret = curl_easy_perform(hnd);
  if(ret != CURLE_OK){
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
    }
  curl_easy_cleanup(hnd);
  hnd = NULL;
  curl_slist_free_all(slist1);
  curl_global_cleanup();
  return ret;
}



void worker(std::unordered_map<std::string, int>  *ptr) {
    int i = 3;
    int ret;
    std::stringstream ss;
    std::string data;
    try {
        while(true) {
            sleep(5);
            ss.str(std::string()); 

            m.lock();
            for (auto &e: *ptr) {
             ss <<  e.first << ", " << e.second << std::endl;
            }
            ptr->clear();
            m.unlock();

            data = ss.str();
            ret = sendMetrics(&data);
            if (ret != 0) {
                 ret = sendMetrics(&data);  //try to send again
            }
            // i--;
           // if ( i == 0) throw std::exception(); //to simulatee an exception
        }
    }catch (...){
       std::cout << "Exception in metric sender thread" << std::endl;
    }
    thread_done = true;
}




int main()
{
    std::string ManagedObjects[4] = {"Huawei XPTO1", "Ericsson EEI32", "Nokia 3413A", "AXE RJO01"};
    while (true){
        if (thread_done ){
            thread_done = false;
            std::cout << "---Restarting thread" << std::endl;
            t1 = std::thread(worker,&equipment_counter);
            t1.detach();
        }
        m.lock();
        for ( std::string c: ManagedObjects) {
            equipment_counter[c]++;    
        }
        m.unlock();
        usleep(500);
    }
 
    return 0;

}

