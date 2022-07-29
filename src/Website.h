const char MAIN_page[] PROGMEM = R"=====(

<html>   
<head>  
<meta name="viewport" content="width=device-width, initial-scale=1">  
<title>ESP8266 kết nối WiFi bằng điện thoại</title>  
<style> 
body {text-align: center;}  
Body {  
  font-family: Calibri, Helvetica, sans-serif;  
  background-color: rgb(255, 255, 255);  
}  
button {   
        background-color: rgb(84, 127, 182);   
        width: 20%;  
        color: rgb(255, 255, 255);   
        padding: 5px;   
        margin: 10px 0px;   
        border: none;   
        cursor: pointer;   
         }   
 form {   
        border: 10px solid rgb(255, 255, 255);  
         
    }   
 input[type=text], input[type=password] {   
        width: 50%;   
        margin: 8px 0;  
        padding: 12px 10px;   
        display: inline-block;   
        border: 2px solid rgb(84, 127, 182);
        box-sizing: border-box;  
         
    }  
 button:hover {   
        opacity: 0.7;   
    }   
  
        
     
 .container {   
        padding: 10px;   
        background-color: lightblue;  
    }
  .textLogin {   
        padding: 0px;   
        background-color: rgb(84, 127, 182); 
        color: rgb(255, 255, 255); 
        margin: 8px 0;  
        border: 10px solid rgb(255, 255, 255); 
    }    
</style>   
</head>    
<body>
    <div class="textLogin">
      <center> <h2> WIFI LOGIN </h2> </center>  
    </div>
     
    <form action="setupWiFi" method="post">  
        <div class="container">   
            <label>Username : </label>   
            <input type="text" placeholder="Enter Username" name="WIFI_SSID" required><br>
            <label>&nbsp Password : </label>   
            <input type="password" placeholder="Enter Password" name="WIFI_PASS" required> <br> 
            <button type="submit">   Login</button>  <br>  
        </div>   
    </form>     
</body>     
</html>  
)=====";