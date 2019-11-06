    // stream the request into a buffer
    output_stream os;
    os << req << std::endl;
    std::string req_message = os.get_buffer();
    OutputDebugStringA(req_message.c_str());
    // lambda
    auto filter_start_line = [](const std::string& message)
    {
        // return the first line of a request message
        return message.substr(0, message.find('\r'));
    };
    // get the start-line, user, and the user-agent from the request
    std::string requestLogMessage =
        filter_start_line(req_message);
    std::string user =
        static_cast<std::string>(req[http::field::from]);
    std::string user_agent =
        static_cast<std::string>(req[http::field::user_agent]);
    std::string target =
        static_cast<std::string>(req.target());
    // lambda for preparing, logging, and sending
    // a response
    auto send_response = [&](
        const std::string& result
        , const std::string& content_type
        , const std::string& response_payload
        )
    {
        // prepare a response message
        http::response<http::string_body> res;
        if (result == "bad_request")
            res.result(http::status::bad_request);
        if (result == "not_found")
            res.result(http::status::not_found);
        if (result == "server_error")
            res.result(http::status::internal_server_error);
        if (result == "ok")
            res.result(http::status::ok);
        if (result == "no_content")
            res.result(http::status::no_content);
        res.version(11);
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::date, beast::string_view(date_for_http_response()));
        // for post content_type is "text/html"
        res.set(http::field::content_type, content_type);
        res.keep_alive(req.keep_alive());
        res.content_length(response_payload.length());
        res.body() = response_payload;
        res.prepare_payload();
        // stream the response into a std::string, and filter
        // the response start-line for logging
        os.clear_and_reset_buffer();
        os << res << std::endl;
        std::string res_message = os.get_buffer();
        OutputDebugStringA(res_message.c_str());
        std::string responseLogMessage =
            filter_start_line(res_message);
        // stop the timer and log the HTTP request/response message
        pStructServer->pResponseTimer->stop();
        pStructServer->pServerLogging->store_log(
            *pStructServer->remote_endpoint
            , requestLogMessage
            , responseLogMessage
            , pStructServer->pResponseTimer->elapsed()
            , user
            , user_agent
            , req_message
            , res_message
        );
        // send the response message
        return send(std::move(res));
    };
    // lambda for solely sending a response
    auto send_response_with_prepared_res = [&](
        http::response<http::file_body> res
        )
    {
        // stream the response into a std::string, and filter
        // the response start-line for logging
        os.clear_and_reset_buffer();
        os << res << std::endl;
        std::string res_message = os.get_buffer();
        OutputDebugStringA(res_message.c_str());
        std::string responseLogMessage =
            filter_start_line(res_message);
        // stop the timer and log the HTTP request/response message
        pStructServer->pResponseTimer->stop();
        pStructServer->pServerLogging->store_log(
            *pStructServer->remote_endpoint
            , requestLogMessage
            , responseLogMessage
            , pStructServer->pResponseTimer->elapsed()
            , user
            , user_agent
            , req_message
            , res_message
        );
        // send the response message
        return send(std::move(res));
    };
    // application user_agent:
    const std::string APP_VALUE_USER_AGENT = "Boost.Beast/248";
    // not sure if browsers will keep this string constant over time...
    // Google Chrome browser user_agent:
    const std::string CHROME_VALUE_USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko)";
    // Make sure we can handle the method
    if (req.method() != http::verb::connect &&
        req.method() != http::verb::delete_ &&
        req.method() != http::verb::get &&
        req.method() != http::verb::head &&
        req.method() != http::verb::options &&
        req.method() != http::verb::post &&
        req.method() != http::verb::put &&
        req.method() != http::verb::trace)
        // Returns a bad request response
        return send_response(
            "bad_request"
            , "text/html"
            , "Unknown HTTP-method"
        );
    // Respond to a CONNECT request
    if (req.method() == http::verb::connect) {
        OutputDebugString(L"-> CONNECT message received\n");
        // not implemented yet
    }
    // Respond to a DELETE request
    if (req.method() == http::verb::delete_) {
        OutputDebugString(L"-> DELETE message received\n");
        // not implemented yet
    }
    // Respond to a GET request
    if (req.method() == http::verb::get) {
        OutputDebugString(L"-> GET message received\n");
        if (user_agent == APP_VALUE_USER_AGENT)
        {
            OutputDebugString(L"-> from a C++ application\n");
            std::string path = path_cat(doc_root, req.target());
            // Request path must be absolute and not contain "..".
            if (req.target().empty() ||
                req.target()[0] != '/' ||
                req.target().find("..") != beast::string_view::npos)
                return send_response(
                    "bad_request"
                    , "text/html"
                    , "Illegal request-target"
                );
            if (req.target().back() == '/')
                path.append("index.html");
            // Attempt to open the file
            beast::error_code ec;
            http::file_body::value_type body;
            body.open(path.c_str(), beast::file_mode::scan, ec);
            // Handle the case where the file doesn't exist
            if (ec == beast::errc::no_such_file_or_directory)
                return send_response(
                    "not_found"
                    , "text/html"
                    , "The resource '" + static_cast<std::string>(req.target()) + "' was not found."
                );
            // Handle an unknown error
            if (ec)
                return send_response(
                    "server_error"
                    , "text/html"
                    , "An error occurred: '" + std::string(ec.message()) + "'"
                );
            // Cache the size since we need it after the move
            auto const size = body.size();
            http::response<http::file_body> res{
                std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(http::status::ok, req.version()) };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, mime_type(path));
            res.content_length(size);
            res.keep_alive(req.keep_alive());
            return send_response_with_prepared_res(std::move(res));
        }
        user_agent = user_agent.substr(0, user_agent.find(" Chrome/"));
        if (user_agent == CHROME_VALUE_USER_AGENT)
        {
            OutputDebugString(L"-> from a browser\n");
            std::string s = req.target().to_string();
            if (s.find("user_space") == std::string::npos)
            {
                OutputDebugString(L"-> for rendering browser layout\n");
                std::string path = path_cat(doc_root, req.target());
                // Request path must be absolute and not contain "..".
                if (req.target().empty() ||
                    req.target()[0] != '/' ||
                    req.target().find("..") != beast::string_view::npos)
                    return send_response(
                        "bad_request"
                        , "text/html"
                        , "Illegal request-target"
                    );
                if (req.target().back() == '/')
                    path.append("index.html");
                // Attempt to open the file
                beast::error_code ec;
                http::file_body::value_type body;
                body.open(path.c_str(), beast::file_mode::scan, ec);
                // Handle the case where the file doesn't exist
                if (ec == beast::errc::no_such_file_or_directory)
                    return send_response(
                        "not_found"
                        , "text/html"
                        , "The resource '" + static_cast<std::string>(req.target()) + "' was not found."
                    );
                // Handle an unknown error
                if (ec)
                    return send_response(
                        "server_error"
                        , "text/html"
                        , "An error occurred: '" + std::string(ec.message()) + "'"
                    );
                // Cache the size since we need it after the move
                auto const size = body.size();
                http::response<http::file_body> res{
                    std::piecewise_construct,
                    std::make_tuple(std::move(body)),
                    std::make_tuple(http::status::ok, req.version()) };
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, mime_type(path));
                res.content_length(size);
                res.keep_alive(req.keep_alive());
                return send_response_with_prepared_res(std::move(res));
            }
        }
        //// a GET request can mean three things
        //// 1) a download request for a file from an app.
        //// 2) a download request for a file within the user space
        ////    from a browser
        //// 3) a download request for a file within the server space
        ////    from a browser (a file that renders the browser's layout)
        //if (user_agent == APP_VALUE_USER_AGENT)
        //{
        //  // its a request from an app, for a file download (1)
        //}
        //// cutoff user_agent string to a point where it matches
        //// the const value for a browser, there is no differentiation
        //// anymore between a Chrome and an Edge browser
        //user_agent = user_agent.substr(0, user_agent.find(" Chrome/"));
        //if (user_agent == CHROME_VALUE_USER_AGENT)
        //{
        //  // its a request from a browser, for a file download within the user space (2)
        //  // if the req.target contains the substring:
        //  // "/server_space/user_space/"
        //  // then a file from within the user_space has to be
        //  // downloaded
        //  // the user_email_address from the user, who downloads
        //  // with a browser, comes in the cookie field of the GET
        //  // so the user can be logged
        //  std::string user =
        //      static_cast<std::string>(req[http::field::cookie]);
        //  std::string path = "";
        //  std::string s = req.target().to_string();
        //  std::string t = "/server_space";
        //  if (s.find("user_space") < std::string::npos)
        //  {
        //      // the string 's' contains: "/server_space/user_space",
        //      // so it's a browser request for a file download
        //      // strip the "/server_space" part of the string
        //      s = s.substr(t.length(), s.length() - t.length());
        //      path = path_cat(doc_root, s);
        //  }
        //}
        //// whats left is a request from a browser for a rendering file
        //// inside the server_space
        //path = path_cat(doc_root, req.target());
        //// Request path must be absolute and not contain "..".
        //if (req.target().empty() ||
        //  req.target()[0] != '/' ||
        //  req.target().find("..") != beast::string_view::npos)
        //  return send_response(
        //      "bad_request"
        //      , "text/html"
        //      , "Illegal request-target"
        //  );
        //if (req.target().back() == '/')
        //  path.append("index.html");
        //// Attempt to open the file
        //beast::error_code ec;
        //http::file_body::value_type body;
        //body.open(path.c_str(), beast::file_mode::scan, ec);
        //// Handle the case where the file doesn't exist
        //if (ec == beast::errc::no_such_file_or_directory)
        //  return send_response(
        //      "not_found"
        //      , "text/html"
        //      , "The resource '" + static_cast<std::string>(req.target()) + "' was not found."
        //  );
        //// Handle an unknown error
        //if (ec)
        //  return send_response(
        //      "server_error"
        //      , "text/html"
        //      , "An error occurred: '" + std::string(ec.message()) + "'"
        //  );
        //// Cache the size since we need it after the move
        //auto const size = body.size();
        //http::response<http::file_body> res{
        //  std::piecewise_construct,
        //  std::make_tuple(std::move(body)),
        //  std::make_tuple(http::status::ok, req.version()) };
        //res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        //res.set(http::field::content_type, mime_type(path));
        //res.content_length(size);
        //res.keep_alive(req.keep_alive());
        //return send_response_with_prepared_res(std::move(res));
    }
    // Respond to a HEAD request
    if (req.method() == http::verb::head) {
        OutputDebugString(L"-> HEAD message received\n");
        // not implemented yet
    }
    // Respond to a OPTIONS request
    if (req.method() == http::verb::options) {
        OutputDebugString(L"-> OPTIONS message received\n");
        // not implemented yet
    }
    // Respond to a POST request
    if (req.method() == http::verb::post) {
        OutputDebugString(L"-> POST message received\n");
        // a POST request can mean two things
        // 1) a request to access the server, from an app. or a browser
        // 2) a file upload from a browser into the user space
        // check if the POST message is concerning a login
        if (target == "/login"
            || target == "/register"
            || target == "/register_confirm"
            || target == "/reset_password"
            || target == "/reset_password_confirm"
            )
        {
            std::string user_email_address = "";
            std::string user_password = "";
            std::string user_code = "";
            // filter the user_email_address, user_password, and
            // the user_code from the request
            filter_email_password_code(req
                , user_email_address
                , user_password
                , user_code
            );
            std::string response_payload = target + ": ";
            // remove forward slash from response_payload
            response_payload.erase(0, 1);
            // check which handler suits the target
            if (target == "/login")
            {
                HandlerForLogin handlerForLogin;
                handlerForLogin.handle_login(
                    user_email_address
                    , user_password
                    , pStructServer->pSqlite
                    , response_payload
                );
            }
            if (target == "/register")
            {
                pStructServer->pHandlerForRegister->handle_register(
                    user_email_address
                    , user_password
                    , pStructServer->pSqlite
                    , response_payload
                );
            }
            if (target == "/register_confirm")
            {
                pStructServer->pHandlerForRegister->handle_register_confirm(
                    user_email_address
                    , user_password
                    , user_code
                    , pStructServer->pSqlite
                    , response_payload
                );
            }
            if (target == "/reset_password")
            {
                pStructServer->pHandlerForResetPassword->handle_reset_password(
                    user_email_address
                    , user_password
                    , pStructServer->pSqlite
                    , response_payload
                );
            }
            if (target == "/reset_password_confirm")
            {
                pStructServer->pHandlerForResetPassword->handle_reset_password_confirm(
                    user_email_address
                    , user_password
                    , user_code
                    , pStructServer->pSqlite
                    , response_payload
                );
            }
            // send the response message
            return send_response(
                "ok"
                , "text/html"
                , response_payload
            );
        }
        // a file upload from a browser into the user space
        // save the file upload to disk
        // TODO ...
        // send the response message
        return send_response(
            "no_content"
            , "text/html"
            , ""
        );
    }
    // Respond to a PUT request
    if (req.method() == http::verb::put) {
        OutputDebugString(L"-> PUT message received\n");
    }
    // Respond to a TRACE request
    if (req.method() == http::verb::trace) {
        OutputDebugString(L"-> TRACE message received\n");
        // the request is echoed back to the client,
        // inside the payload of the response
        // in this application a non-standard comment is added
        http::string_body::value_type response_payload;
        response_payload = std::string("server is alive\r\n")
            + req_message;
        // send the response message
        return send_response(
            "ok"
            , "message/http"
            , response_payload
        );
    }
