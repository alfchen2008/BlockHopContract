
var express = require("express");
var XMLHttpRequest = require("xmlhttprequest").XMLHttpRequest;
var app = express();  
var server = require("http").createServer(app);
var Eos = require("eosjs")

app.use(express.static("public"));

app.all('*', function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Headers", "Content-Type,Content-Length, Authorization, Accept,X-Requested-With");
    res.header("Access-Control-Allow-Methods","PUT,POST,GET,DELETE,OPTIONS");
    res.header("X-Powered-By",' 3.2.1')
    if(req.method=="OPTIONS") res.send(200);/*让options请求快速返回*/
    else  next();
});

app.get("/", function(req, res){
    res.send('Hello World');
})

// init
var config = {
    chainId: "cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f", // 32 byte (64 char) hex string
    keyProvider: ['5Jy2rdaffdCSP2UYXbNEQCoxvhT4M6hpAaquZ3tNC7YTcrmqPe7'], // WIF string or array of keys..
    httpEndpoint: 'http://10.101.1.176:8888',
    // mockTransactions: () => 'pass', // or 'fail'
    // transactionHeaders: (expireInSeconds, callback) => {
    //   callback(null/*error*/, headers)
    // },
    expireInSeconds: 60,
    broadcast: true,
    debug: false, // API and transactions
    sign: true
}
var eos = Eos(config);

var xmlRequest = function() {
    var data = JSON.stringify({
        "scope": "klc",
        "code": "klc",
        "table": "address",
        "json": "true"
      });
      
      var xhr = new XMLHttpRequest();
      xhr.withCredentials = true;
      
      xhr.addEventListener("readystatechange", function () {
        if (this.readyState === this.DONE) {
          console.log(this.responseText);
        }
      });
      
      xhr.open("POST", "http://10.101.1.158:8888/v1/chain/get_table_rows");
      
      xhr.send(data);
}

// 测试
app.get("/test", function(req, res) {
    var param = req.query.param;
    console.log("param = " + param);

    var timestamp = Date.parse(new Date());
    eos.transfer('user1', 'adventure', '2.0000 FOD', timestamp);
});

// getinfo
app.get("/getInfo", function(req, res) {
    eos.getInfo({}).then(result => {
        console.log(result);
    });
});

// contract
app.get("/contract", function(req, res) {
    var contractObj;
    eos.contract("klc").then(result => {
        contractObj = result;
        console.log(JSON.stringify(contractObj))

        contractObj.myaction('lkjhg', {sign: true, authorization: 'klc'}).then((result => {
            console.log("klc result = " + JSON.stringify(result));
        }));
        console.log("_______________________创建成功");
    });
});

// 购买
app.get("/clickBuy", function(req, res){

    var id = req.query.id;
    console.log("param = " + id);
    eos.transfer('user1', 'adventure', '2.0000 SYS', id);

    res.send("success");
});

// 开始冒险
app.get("/explore", function(req, res){
    // var timestamp = Date.parse(new Date());
    var timestamp = req.query.exploreIndex;
    console.log("timestamp = " + timestamp);
    eos.transfer('user1', 'adventure', '2.0000 FOD', timestamp);
});


console.log("server is running!");
server.listen(8080);