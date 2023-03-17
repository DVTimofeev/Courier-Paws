#include "api_handler.h"

namespace api_handler{

ApiHandler::ApiHandler(Application& app) 
        : app_{app}{
    LinkJoinWithoutAuthorize();
    LinkPlayerList();
    LinkGameState();
    if (app_.IsTickRequestAllowed()) {
        LinkGameTick();
    }
    LinkPlayerAction();
}

std::vector<std::string> ApiHandler::GetParsedTarget(std::string_view target) const{
    std::vector<std::string> parsed_target;
    for (auto&& ch : target)
    {
        if (ch == '/'){
            parsed_target.emplace_back();
        } else {
            parsed_target.back().push_back(ch);
        } 
    }
    return parsed_target;
    
}   

bool ApiHandler::IsApiRequest(const StringRequest& request) const{
    auto parsed_target = GetParsedTarget(request.target());
    if(parsed_target.size() > 2 && parsed_target[0] == "api") return true;
    return false;
}

StringResponse ApiHandler::HandleRequest(const StringRequest& request){
    auto parsed_target = GetParsedTarget(request.target());
    
    if (parsed_target[1] == "v1") {
        if (parsed_target[2] == "maps") {
            return HandleMapsRequest(request);
        } else {
            return uri_handler_.Process(request);
        } 
    }
    StringResponse response;
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);
    response.result(http::status::bad_request);
    std::string string_body = R"({"code":"badRequest", "message":"Wrong version of api"})";
    response.body() = string_body;
    response.content_length(string_body.size());
    return response;
}

StringResponse ApiHandler::HandleMapsRequest(const StringRequest& request){
    auto parsed_target = GetParsedTarget(request.target());
    StringResponse response;
    std::string json_body;
    if (request.method() == http::verb::get || request.method() == http::verb::head) {
        if (parsed_target.size() == 3){
            json_body = app_.GetMapsSpec();   
        } else if ( auto id = Map::Id(parsed_target[3]); auto map = app_.FindMap(id)){
            json_body = app_.GetMapInfo(*map);
        } else {
            json_body = R"({"code": "mapNotFound", "message": "Map not found"})"; 
            response.result(http::status::not_found);
        }  
    } else {
        response.result(http::status::method_not_allowed);
        response.set(http::field::allow, "GET, HEAD");
        json_body = R"({"code": "invalidMethod", "message": "Only GET, HEAD method is expected"})"; 
    }

    response.body() = json_body;
    response.content_length(json_body.size());
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);
    return response;
}

StringResponse ApiHandler::AddPlayer(std::string_view body) {
    StringResponse response;
    response.result(http::status::ok);
    json::value req_body;
    std::string string_body;
    req_body = json::parse(std::string(body));
    auto join_game_result = app_.JoinGame(  
        req_body.at("mapId").as_string().c_str(), 
        req_body.at("userName").as_string().c_str()
    ); 
    string_body = json::serialize(
        json::value{
            {"authToken", *join_game_result.token},
            {"playerId", join_game_result.player_id}
        }
    );   
    response.body() = string_body;
    response.content_length(string_body.size());
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);
    return response;
}

StringResponse ApiHandler::GetPlayers(const Token& token, std::string_view body) {
    StringResponse response;
    response.result(http::status::ok);
    std::string string_body;
    auto player = app_.FindPlayer(token);
    string_body = app_.ListPlayers(*player);
    response.body() = string_body;
    response.content_length(string_body.size());
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);
    return response;
}

StringResponse ApiHandler::GetGameState(const Token& token, std::string_view body) {
    StringResponse response;
    response.result(http::status::ok);
    std::string string_body;
    string_body = app_.GetGameState(token);
    response.body() = string_body;
    response.content_length(string_body.size());
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);
    return response;
}

StringResponse ApiHandler::UpdateGameState(std::string_view body) {
    StringResponse response;
    response.result(http::status::ok);
    std::string string_body;
    json::value json_body;
    auto is_parsed = false;
    try {
        json_body = json::parse(std::string(body));
        is_parsed = true;
    } catch (...) {
        
    }
    if (is_parsed && json_body.as_object().contains("timeDelta") && json_body.at("timeDelta").is_int64() && json_body.at("timeDelta").as_int64() > 0) {
        std::chrono::milliseconds delta(json_body.at("timeDelta").as_int64());
        app_.UpdateGame(delta);
        string_body = "{}";
    } else {
        response.result(http::status::bad_request);
        string_body = R"({"code":"invalidArgument", "message":"Failed to parse tick request JSON"})";
    }
    response.body() = string_body;
    response.content_length(string_body.size());
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);
    return response;
}

StringResponse ApiHandler::SetPlayerAction(const Token& token, std::string_view body) {
    StringResponse response;
    response.result(http::status::ok);
    std::string string_body;
    json::value json_body;
    auto is_parsed = false;
    try {
        json_body = json::parse(std::string(body));
        is_parsed = true;
    } catch (...) {
    
    }
    if (is_parsed && json_body.as_object().contains("move") && ValidatePlayerMove(json_body.at("move").as_string().c_str())) {
        string_body = app_.MovePlayer(token, std::string(json_body.at("move").as_string().c_str()));
    } else {
        response.result(http::status::bad_request);
        string_body = R"({"code":"invalidArgument", "message":"Failed to parse action"})";
    }
    response.body() = string_body;
    response.content_length(string_body.size());
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);
    return response;
}

bool ApiHandler::ValidatePlayerMove(const std::string_view& move) {
    std::string VALID_MOVE = " LRUD";
    if (move.empty() || (move.size() == 1 && VALID_MOVE.find(move))) {
        return true;
    } else {
        return false;
    }
}

// LINK URI_API

void ApiHandler::LinkJoinWithoutAuthorize() {
    auto ptr = uri_handler_.AddEndpoint(Endpoint::JOIN_GAME);
    if(ptr) {
        ptr->SetNeedAuthorisation(false)
            .SetAllowedMethods({http::verb::post}, ErrorMessage::POST_IS_EXPECTED, MiscMessage::ALLOWED_POST_METHOD)
            .SetProcessFunction([&](std::string_view body){
                return AddPlayer(body);
            });
    }
}

void ApiHandler::LinkPlayerList() {
    auto ptr = uri_handler_.AddEndpoint(Endpoint::PLAYER_LIST);
    if(ptr) {
        ptr->SetNeedAuthorisation(true)
            // .SetAllowedMethods({http::verb::post}, ErrorMessage::POST_IS_EXPECTED, MiscMessage::ALLOWED_POST_METHOD)
            .SetAllowedMethods({http::verb::get, http::verb::head}, ErrorMessage::GET_IS_EXPECTED, MiscMessage::ALLOWED_GET_HEAD_METHOD)
            .SetProcessFunction([&](const Token& token, std::string_view body){
                return GetPlayers(token, body);
            });
    }
}

void ApiHandler::LinkGameState() {
    auto ptr = uri_handler_.AddEndpoint(Endpoint::GAME_STATE);
    if(ptr) {
        ptr->SetNeedAuthorisation(true)
            // .SetAllowedMethods({http::verb::post}, ErrorMessage::POST_IS_EXPECTED, MiscMessage::ALLOWED_POST_METHOD)
            .SetAllowedMethods({http::verb::get, http::verb::head}, ErrorMessage::GET_IS_EXPECTED, MiscMessage::ALLOWED_GET_HEAD_METHOD)
            .SetProcessFunction([&](const Token& token, std::string_view body){
                return GetGameState(token, body);
            });
    }
}

void ApiHandler::LinkGameTick() {
    auto ptr = uri_handler_.AddEndpoint(Endpoint::GAME_TICK);
    if(ptr) {
        ptr->SetNeedAuthorisation(false)
            .SetAllowedMethods({http::verb::post}, ErrorMessage::POST_IS_EXPECTED, MiscMessage::ALLOWED_POST_METHOD)
            .SetProcessFunction([&](std::string_view body){
                return UpdateGameState(body);
            });
    }
}

void ApiHandler::LinkPlayerAction() {
    auto ptr = uri_handler_.AddEndpoint(Endpoint::PLAYER_ACTION);
    if(ptr) {
        ptr->SetNeedAuthorisation(true)
            .SetAllowedMethods({http::verb::post}, ErrorMessage::POST_IS_EXPECTED, MiscMessage::ALLOWED_POST_METHOD)
            .SetProcessFunction([&](const Token& token, std::string_view body){
                return SetPlayerAction(token, body);
            });
    }
}

// JSON parsing

json::array ApiHandler::parse_roads(const std::vector<Road>& roads) const{
    json::array result;
    for (auto&& road : roads){
        json::object j;
        auto start = road.GetStart();
        auto end = road.GetEnd();
        j.emplace("x0", start.x);
        j.emplace("y0", start.y);
        if (road.IsHorizontal()){
            j.emplace("x1", end.x);
        } else {
            j.emplace("y1", end.y);
        }
        result.push_back(j);
    }
    return result;
}
json::array ApiHandler::parse_buildings(const std::vector<Building>& buildings) const{
    json::array result;
    for (auto&& building : buildings){
        json::object j;
        auto bounds = building.GetBounds();
        j.emplace("x", bounds.position.x);
        j.emplace("y", bounds.position.y);
        j.emplace("w", bounds.size.width);
        j.emplace("h", bounds.size.height);
        result.push_back(j);
    }
    return result;
}

json::array ApiHandler::parse_offices(const std::vector<Office>& offices) const{
    json::array result;
    for (auto&& office : offices){
        json::object j;
        auto pos = office.GetPosition();
        auto offset = office.GetOffset();
        j.emplace("id", *office.GetId());
        j.emplace("x", pos.x);
        j.emplace("y", pos.y);
        j.emplace("offsetX", offset.dx);
        j.emplace("offsetY", offset.dy);
        result.push_back(j);
    }
    return result;
}

} // namespace api_handler