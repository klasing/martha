//****************************************************************************
//*      	            global
//****************************************************************************
//var cookieIsSet = false;
//var user_email_address = "";
var user_password = "";
//****************************************************************************
//*      	            openRegister
//****************************************************************************
function openRegister()
{
  console.log("openRegister()");
  window.open("popup-register.html", "_blank", settingsForSizeAndPosition(400, 400));
}
//****************************************************************************
//*      	            openLogin
//****************************************************************************
function openLogin()
{
  console.log("openLogin()");
  window.open("popup-login.html", "_blank", settingsForSizeAndPosition(400, 400));
}
//****************************************************************************
//*      	            settingsForSizeAndPosition
//****************************************************************************
function settingsForSizeAndPosition(width, height)
{
  var x = window.screenLeft + (window.outerWidth / 2 - width / 2);
  var y = window.screenTop + (window.outerHeight / 2 - height / 2);
  var settings =
    "width=" + width + "," +
    "height=" + height + "," +
    "left=" + x + "," +
    "top=" + y;
  return settings
}
//****************************************************************************
//*      	            getCookie
//****************************************************************************
function getCookie(cname)
{
  console.log("getCookie()");
  var name = cname + "=";
  var decodedCookie = decodeURIComponent(document.cookie);
  var ca = decodedCookie.split(';');
  for (var i = 0; i < ca.length; i++)
  {
    var c = ca[i];
    while (c.charAt(0) == ' ')
    {
      c = c.substring(1);
    }
    if (c.indexOf(name) == 0)
    {
      return c.substring(name.length, c.length);
    }
  }
}
//****************************************************************************
//*      	            checkCookie
//****************************************************************************
function checkCookie()
{
  console.log("checkCookie()");
  user_email_address_cookie = getCookie("user_email_address");
  console.log("user_email_address_cookie: " + user_email_address_cookie);
  if (typeof user_email_address_cookie === 'undefined')
  {
	console.log("no cookie set");
    document.getElementById("login_user_email_address").innerHTML =
      "";
    //window.cookieIsSet = false;
    localStorage.setItem('cookieIsSet', 'false');
    localStorage.setItem('user_email_address', '');
  }
  else
  {
	console.log("cookie available");
    document.getElementById("login_user_email_address").innerHTML =
      "logged in as: " + user_email_address_cookie;
    //window.cookieIsSet = true;
    localStorage.setItem('cookieIsSet', 'true');
    localStorage.setItem('user_email_address', user_email_address_cookie);
  }
}
//****************************************************************************
//*      	            openDocument
//****************************************************************************
function openDocument(document)
{
  console.log("openDocument()");
  window.open(document, "frame-center-bottom");
}
//****************************************************************************
//*      	            set_href_and_activate
//****************************************************************************
function set_href_and_activate()
{
  console.log("set_href_and_activate");
  if (localStorage.getItem('cookieIsSet') == 'false')
  {
	  document.getElementById("msg").innerHTML =
	    "Please login first";
	  return;
  }
  else
  {
	  document.getElementById("msg").innerHTML =
	    "";
  }
  var input = document.getElementById("input").value;
  if (input == "")
  {
    console.log("no input");
    return;
  }
  var hyper_link = document.getElementById("hyper_link");
  hyper_link.setAttribute("href", "user_space/" + input);
  hyper_link.click();
}
//****************************************************************************
//*      	            setCookie
//****************************************************************************
function setCookie(cname, cvalue, exdays)
{
  console.log("setCookie()");
  var d = new Date();
  d.setTime(d.getTime() + (exdays*24*60*60*1000));
  var expires = "expires=" + d.toGMTString();
  document.cookie = cname + "=" + cvalue + ";" + expires;
}
//****************************************************************************
//*      	            doLogin
//****************************************************************************
function doLogin()
{
  console.log("doLogin()");
  // get the values from a form, to create the payload for a POST request
  window.user_email_address = document.getElementById("user_email_address").value;
  window.user_password = document.getElementById("user_password").value;
  var payload_for_post = "user_email_address=" + window.user_email_address + "&" + "user_password=" + window.user_password;
  console.log("payload_for_post: " + payload_for_post);
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function()
  {
    if (this.readyState == 4 && this.status == 200)
    {
      var login_result = this.responseText;
      document.getElementById("login_result").innerHTML = login_result;

      if (login_result == "login: succeeded")
      {
        // set cookie, for one day
        setCookie("user_email_address", window.user_email_address, 1);
        // set login_user_email_address in parent window
        window.opener.document.getElementById("login_user_email_address").innerHTML =
          "logged in as: " + window.user_email_address;
        localStorage.setItem('user_email_address', window.user_email_address);
      }
    }
  }
  xhttp.open("POST", "http://" + window.location.hostname + ":8080/login", true);
  xhttp.setRequestHeader("From", window.user_email_address);
  xhttp.send(payload_for_post);
}
//****************************************************************************
//*      	            doRegister
//****************************************************************************
function doRegister()
{
  console.log("doRegister()");
  // get the values from a form, to create the payload for a POST request
  window.user_email_address = document.getElementById("user_email_address").value;
  window.user_password = document.getElementById("user_password").value;
  var payload_for_post = "user_email_address=" + window.user_email_address + "&" + "user_password=" + window.user_password;
  console.log("payload_for_post: " + payload_for_post);
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function()
  {
    if (this.readyState == 4 && this.status == 200)
    {
      document.getElementById("register_result").innerHTML = this.responseText;
      if (this.responseText == "register: enter the code received by email")
      {
        // hide the fist 'submit' button
        document.getElementById("button_register").style.display = "none";
        // show the 'register_confirm' form
        document.getElementById("register_confirm").style.display = "block";
      }
    }
  }
  xhttp.open("POST", "http://" + window.location.hostname + ":8080/register", true);
  xhttp.setRequestHeader("From", localStorage.getItem('user_email_address'));
  xhttp.send(payload_for_post);
}
//****************************************************************************
//*      	            doResetPassword
//****************************************************************************
function doResetPassword()
{
  console.log("doResetPassword()");
  // get the values from a form, to create the payload for a POST request
  window.user_email_address = document.getElementById("user_email_address").value;
  window.user_password = document.getElementById("user_password").value;
  var payload_for_post = "user_email_address=" + window.user_email_address + "&" + "user_password=" + window.user_password;
  console.log("payload_for_post: " + payload_for_post);
  if (user_email_address == "guest@example.com")
  {
    document.getElementById("reset_password_result").innerHTML = "you can not reset the password from the default user";
    return;
  }
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function()
  {
    if (this.readyState == 4 && this.status == 200)
    {
      document.getElementById("reset_password_result").innerHTML = this.responseText;
      if (this.responseText == "reset_password: enter the code received by email")
      {
        // hide the fist 'submit' button
        document.getElementById("button_reset_password").style.display = "none";
        // show the 'reset_password_confirm' form
        document.getElementById("reset_password_confirm").style.display = "block";
      }
    }
  }
  xhttp.open("POST", "http://" + window.location.hostname + ":8080/reset_password", true);
  xhttp.setRequestHeader("From", localStorage.getItem('user_email_address'));
  xhttp.send(payload_for_post);
}
//****************************************************************************
//*      	            doConfirm
//****************************************************************************
function doConfirm(result, butt, target)
{
  console.log("doConfirm()");
  // get the value from the 'code_confirm' field, to create the payload for a POST request
  var code_confirm = document.getElementById("code_confirm").value;
  var payload_for_post = "user_email_address=" + window.user_email_address + "&" + "user_password=" + window.user_password + "&" + "code_confirm=" + code_confirm;
  console.log("payload_for_post: " + payload_for_post);
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function()
  {
    if (this.readyState == 4 && this.status == 200)
    {
      document.getElementById(result).innerHTML = this.responseText;
    }
  }
  xhttp.open("POST", "http://" + window.location.hostname + ":8080/" + target, true);
  xhttp.setRequestHeader("From", localStorage.getItem('user_email_address'));
  xhttp.send(payload_for_post);
  // hide the second 'submit' button
  document.getElementById(butt).style.display = "none";
}
//****************************************************************************
//*      	            get_user_email_address
//****************************************************************************
function get_user_email_address()
{
	return localStorage.getItem('user_email_address');
}
