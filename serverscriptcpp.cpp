#include <drogon/drogon.h>
#include <nlohmann/json.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/torrent_status.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <thread>
#include <chrono>

using json = nlohmann::json;
using namespace drogon;

void printIfDebuggerPresent(const std::string& output) {
    //std::ifstream wchan("/proc/self/status");
    //if (!wchan.is_open()) {
    //    std::cerr << "Failed to open /proc/self/status" << std::endl;
    //    return;
    //}

    //std::string line;
    //while (std::getline(wchan, line)) {
    //    if (line.find("TracerPid") != std::string::npos) {
    //        // If TracerPid is not 0, a debugger is present
    //        if (line.find(": 0") == std::string::npos) {
                std::cout << output << std::endl;
    //        }
    //        break;
    //    }
    //}
}

using json = nlohmann::json;
using namespace drogon;
namespace lt = libtorrent;

void download_torrent(const std::string& infohash) {
    // Create a session
    lt::settings_pack settings;
    lt::session session(settings);
    printIfDebuggerPresent("12");

    lt::add_torrent_params torrent_params;
    torrent_params.save_path = "./"; // Save path for downloaded files
    printIfDebuggerPresent("13");

    std::string res_url;

    printIfDebuggerPresent("infohash in download function");
    printIfDebuggerPresent(infohash);

    torrent_params.url = "magnet:?xt=urn:btih:" + infohash;

    std::string rurl = "magnet:?xt=urn:btih:" + infohash;
    printIfDebuggerPresent(rurl);

    //size_t pos1 = infohash.find('=');
    //if (pos1 != std::string::npos) {
    //    // Find the second equals sign
    //    size_t pos2 = infohash.find('=', pos1 + 1);
    //    if (pos2 != std::string::npos) {
    //        // Truncate the string after the second equals sign
    //        res_url = infohash.substr(0, pos2);
    //    }
    //}
    //printIfDebuggerPresent("res url");
    //printIfDebuggerPresent(res_url);

    //torrent_params.url = res_url;
    
    printIfDebuggerPresent("14");

    // Add the torrent to the session
    lt::torrent_handle handle = session.add_torrent(std::move(torrent_params));
    printIfDebuggerPresent("15");

    // Downloading is asynchronous, so we need to wait and check the progress.
    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
	printIfDebuggerPresent("16");

        // Get torrent status
        lt::torrent_status status = handle.status();

        // Check if download is complete
        if (status.is_finished) {
            std::cout << "Download complete" << std::endl;
            break;
        }

        // Print progress
        std::cout << "Downloading: " << status.progress_ppm / 10000 << "%" << std::endl;
	printIfDebuggerPresent("17");
    }

    // Save the torrent file
    std::ofstream out(infohash + ".torrent", std::ios_base::binary);
    handle.save_resume_data(lt::torrent_handle::save_info_dict);

    auto const resume_data = handle.write_resume_data();

    const std::type_info& ti = typeid(resume_data);

    printIfDebuggerPresent(ti.name());
}


void searchProwlarr(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, const std::string& apiKey) {

    printIfDebuggerPresent("1");
    auto queries = req->getParameter("queries"); // Assuming queries are sent as a comma-separated string
    std::stringstream ss(queries);
    std::string query;
    std::vector<json> results;

    auto client = HttpClient::newHttpClient("http://127.0.0.1:9696");
    printIfDebuggerPresent("2");
  
    printIfDebuggerPresent("queries" + queries);

    printIfDebuggerPresent("ss: " + ss.str());

    while (std::getline(ss, query, ',')) {
	printIfDebuggerPresent("2.1");

    // Creating a new HTTP GET request
    auto request = HttpRequest::newHttpRequest();
	printIfDebuggerPresent("2.2");

    request->setMethod(drogon::Get); // Correctly setting the method as a GET request
    request->addHeader("X-API-Key", apiKey);
    request->setPath("/api/v1/search");
    request->setParameter("q", query);

        printIfDebuggerPresent("3");

        client->sendRequest(request, [query, &results, callback, &ss](ReqResult result, const HttpResponsePtr& response) {
            if (result == ReqResult::Ok) {

                printIfDebuggerPresent("4");

                auto torrents = json::parse(response->getBody());
                printIfDebuggerPresent("6");

                // Sort and get top 5 torrents based on seeders
                std::sort(torrents.begin(), torrents.end(), [](const json& a, const json& b) {
                    return a["seeders"].get<int>() > b["seeders"].get<int>();
                    printIfDebuggerPresent("7");
                });
                printIfDebuggerPresent("8");

                if (torrents.size() > 5) {
	
		    printIfDebuggerPresent("8.5 torrent size > 5");

                    torrents.erase(torrents.begin() + 5, torrents.end());
                }
                printIfDebuggerPresent("9");
                results.push_back({torrents});
                printIfDebuggerPresent("torrent size ");
		printIfDebuggerPresent(std::to_string(torrents.size()));
		
            } else {
		printIfDebuggerPresent("9.5 failed to fetch data");
                results.push_back({{"query", query}, {"error", "Failed to fetch data"}});
            }

	    for(auto result : results){
    		printIfDebuggerPresent("result dump");
	        printIfDebuggerPresent(result.dump());

		for(auto torrent : result){

		    std::string title = torrent["sortTitle"].get<std::string>();
		    if(title == ""){
			printIfDebuggerPresent("title empty");
		    }
		    printIfDebuggerPresent(title);

		    std::string infohash = torrent["infoHash"].get<std::string>();
		    if(infohash == ""){
		        printIfDebuggerPresent("infohash empty");
		    } else {
		        download_torrent(infohash);
		    }
		}
		printIfDebuggerPresent("\n");

                //std::string magnetUrl = result["magnetUrl"].get<std::string>();
		//printIfDebuggerPresent("magnetUrl");
		//printIfDebuggerPresent(magnetUrl);
	    }
               
            printIfDebuggerPresent("10");
	    std::cout << (results.size());

            // Ensure that the lambda captures 'ss' by reference
            //if (results.size() == std::count(ss.str().begin(), ss.str().end(), ',') + 1) {
            //    auto resp = HttpResponse::newHttpResponse();
            //    resp->setContentTypeCode(CT_APPLICATION_JSON);
            //    resp->setBody(json(results).dump());
            //    callback(resp);
            //}
        });
    }
}

int main() {

    app().addListener("127.0.0.1", 5000);

    // app().registerHandler("/search", [](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
    //     searchProwlarr(req, std::move(callback), "9c783af0405849bdad5ad18551ac60af");
    // });

   app().registerHandler(
        "/search",
        [](const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback) {
               searchProwlarr(req, std::move(callback), "9c783af0405849bdad5ad18551ac60af");
        },
        {Post});

    app().registerHandler(
        "/hello",
        [](const HttpRequestPtr &,
           std::function<void(const HttpResponsePtr &)> &&callback) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Hello, World!");
            callback(resp);
        },
        {Get});

    app().registerHandler(
    "/",
    [](const drogon::HttpRequestPtr &,
       std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
        auto resp = drogon::HttpResponse::newFileResponse("/mnt/local/web/videosearch/index.html");
        callback(resp);
    },
    {drogon::Get});

    app().registerHandler(
    "/upload",
    [](const HttpRequestPtr &req,
       std::function<void(const HttpResponsePtr &)> &&callback) {
        auto files = req->getUploadedFiles();
        for (const auto &file : files) {
            // Save each file; for example, save to /mnt/uploads/
            std::string savePath = "/mnt/local/web/pdfuploads/" + file.getFileName();
	    std::cout << savePath << '\n';
            file.saveAs(savePath);
        }
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k200OK);
        resp->setBody("Files uploaded successfully.");
        callback(resp);
    },
    {Post});

    app().run();
    return 0;
}
