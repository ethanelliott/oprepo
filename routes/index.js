var express = require('express');
var crypto = require('crypto');
var mongo = require('mongodb');
var session = require('client-sessions');
var numeral = require('numeral');
var JSZip = require("jszip");

var router = express.Router();

var y = [
  2016,
  2017
];

/* GET home page. */
router.get('/', function(req, res, next) {
  var db = req.dbUser;
	var collection = db.get('user');
  if (req.session && req.session.user){
    var query = collection.find({'username':req.session.user.username}).then(
      function(value){
        if(value){
          res.render('index', { title: 'OPrepo', years:y});
        }else {
          res.render('login', { title: 'Login'});
        }
      });
    }
    else {
      res.render('login', { title: 'Login'});
    }
});
router.get('/r', function(req, res, next) {
  res.redirect('/');
});

router.get('/upload', function(req, res, next) {
  var db = req.dbUser;
	var collection = db.get('user');
  if (req.session && req.session.user){
    var query = collection.find({'username':req.session.user.username}).then(
      function(value){
        if(value){
          res.render('upload', { title: "Upload File"});
        }else {
          res.redirect('/');
        }
      });
    }
    else {
      res.redirect('/');
    }
});
router.post('/r/:year/:id/upload', function(req, res, next) {
  var year = req.params.year;
  var repoID = req.params.id;
  var db = req.dbUser;
  var data = req.body.data;
	var name = req.body.name;
	var collection = db.get('user');
  if (req.session && req.session.user){
    var query = collection.find({'username':req.session.user.username}).then(
      function(value){
        if(value){
          if (y.contains(parseInt(year)))
          {
            var dbD = req.dbData;
            var col = "OPR" + year;
            var yearData = dbD.get(col);
            var newID = generateID();
            yearData.update(
              {_id:repoID},
              {$addToSet:{files:
                {
                  "id":newID,
                  "name":name,
                  "data":data
                }
              }
            }
            ).then(function(){
              res.json({});
            });
          }
          else {
            res.json({});
          }
        }else {
          res.json({});
        }
      });
    }
    else {
      res.json({});
    }
});

router.get('/r/:year', function(req, res, next) {
  var year = req.params.year;
  var db = req.dbUser;
	var collection = db.get('user');
  if (req.session && req.session.user){
    var query = collection.find({'username':req.session.user.username}).then(
      function(value){
        if(value){
          if (y.contains(parseInt(year)))
          {
            var dbD = req.dbData;
            var col = "OPR" + year;
            var yearData = dbD.get(col);
            yearData.find({}).then(function(r){
              if (r.length > 0)
              {
                var d = new Date();
                sortDates(r);
                var recent = r[0];
                var zip = new JSZip();
                r = recent;
                for(i = 0; i < r.files.length; i++)
                {
                  zip.file(r.files[i].name, r.files[i].data, {base64: true});
                }
                zip.generateAsync({type:"base64"}).then(function (Zipbase64) {
                  var oD = new Date(recent.date);
                  var daysInbetween = daysBetween(oD,d);
                  Zipbase64 = "data:application/zip;base64," + Zipbase64;
                  res.render('repo', { title: year, recentUpload:recent, date:oD.toDateString(), time:oD.toLocaleTimeString(), days:daysInbetween, downloadZip:Zipbase64});
                });
              }
              else {
                res.redirect('/');
              }
            });
          }
          else {
            res.redirect('/');
          }
        }else {
          res.redirect('/');
        }
      });
    }
    else {
      res.redirect('/');
    }
});
var daysBetween = function(date1, date2)
{
  var one_day=1000*60*60*24;
  var date1_ms = date1.getTime();
  var date2_ms = date2.getTime();
  var difference_ms = date2_ms - date1_ms;
  return Math.round(difference_ms/one_day);
};

router.get('/r/:year/history', function(req, res, next) {
  var year = req.params.year;
  var db = req.dbUser;
  var collection = db.get('user');
  if (req.session && req.session.user){
    var query = collection.find({'username':req.session.user.username}).then(
      function(value){
        if(value){
          if (y.contains(parseInt(year)))
          {
            var dbD = req.dbData;
            var col = "OPR" + year;
            var yearData = dbD.get(col);
            yearData.find({}).then(function(r){
              if (r.length > 0)
              {
                sortDates(r);
                for(i=0;i<r.length;i++)
                {
                  var cDate = new Date(r[i].date);
                  r[i].date = {
                    "d":cDate.toDateString(),
                    "t":cDate.toLocaleTimeString()
                  };
                }
                res.render('repohistory', { title: year, historyList:r});
              }
            });
          }
          else {
            res.redirect('/');
          }
        }else {
          res.redirect('/');
        }
      });
    }
    else {
      res.redirect('/');
    }
});

router.get('/r/:year/:uploadID', function(req, res, next) {
  var year = req.params.year;
  var uploadID = req.params.uploadID;
  var db = req.dbUser;
	var collection = db.get('user');
  if (req.session && req.session.user){
    var query = collection.find({'username':req.session.user.username}).then(
      function(value){
        if(value){
          if (y.contains(parseInt(year)))
          {
            var dbD = req.dbData;
            var col = "OPR" + year;
            var yearData = dbD.get(col);
            yearData.find({_id:uploadID}).then(function(r){
              if (r.length > 0)
              {
                var d = new Date();
                var recent = r[0];
                var zip = new JSZip();
                r = recent;
                for(i = 0; i < r.files.length; i++)
                {
                  zip.file(r.files[i].name, r.files[i].data, {base64: true});
                }
                zip.generateAsync({type:"base64"}).then(function (Zipbase64) {
                  var oD = new Date(recent.date);
                  var daysInbetween = daysBetween(oD,d);
                  Zipbase64 = "data:application/zip;base64," + Zipbase64;
                  res.render('repo', { title: year, recentUpload:recent, date:oD.toDateString(), time:oD.toLocaleTimeString(), days:daysInbetween, downloadZip:Zipbase64});
                });
              }
            });
          }
          else {
            res.redirect('/');
          }
        }else {
          res.redirect('/');
        }
      });
    }
    else {
      res.redirect('/');
    }
});
router.get('/r/:year/:uploadID/:fileID', function(req, res, next) {
  var year = req.params.year;
  var uploadID = req.params.uploadID;
  var fileID = req.params.fileID;
  var db = req.dbUser;
	var collection = db.get('user');
  if (req.session && req.session.user){
    var query = collection.find({'username':req.session.user.username}).then(
      function(value){
        if(value){
          if (y.contains(parseInt(year)))
          {
            var dbD = req.dbData;
            var col = "OPR" + year;
            var yearData = dbD.get(col);
            yearData.find({'_id':uploadID},{}).then(function(r){
              if (r)
              {
                var fileData;
                for(i=0; i < r[0].files.length; i++)
                {
                  if (r[0].files[i].id === fileID)
                  {
                    fileData = r[0].files[i];
                  }
                }
                if (fileData)
                {
                  res.render('fileview', { title: fileData.name, filedata: fileData, y:year, u: uploadID, f:fileID});
                }
                else
                {
                  res.redirect('/' + year);
                }
              }
              else {
                res.redirect('/');
              }
            });
          }
          else {
            res.redirect('/');
          }
        }else {
          res.redirect('/');
        }
      });
    }
    else {
      res.redirect('/');
    }
});

router.get('/logout', function(req, res) {
  req.session.reset();
  res.redirect('/');
});

router.post('/r/:year/new', function(req, res)
{
  var year = req.params.year;
	var rdate = req.body.oprdate;
	var rmessage = req.body.oprmessage;
  var db = req.dbUser;
	var collection = db.get('user');
  if (req.session && req.session.user){
    var query = collection.find({'username':req.session.user.username}).then(
      function(value){
        if(value){

          if (y.contains(parseInt(year)))
          {
            var dbD = req.dbData;
            var col = "OPR" + year;
            var yearData = dbD.get(col);
            var newRepoFormat = {
              "date":rdate,
              "user":req.session.user.username,
              "message":rmessage,
              "files":[]
            };
            yearData.insert(newRepoFormat, function(err, d){
              res.json({"response":"OK","error":"", "newid":d._id});
            });
          }
          else {
            res.json({"response":"FAIL","error":"invalid year","newid":""});
          }
        }else {
          res.json({"response":"FAIL","error":"invalid user; not authorized","newid":""});
        }
      });
    }
    else {
      res.json({"response":"FAIL","error":"invalid user; not authorized","newid":""});
    }
});

router.post('/ulogin', function(req, res)
{
	var db = req.dbUser;
  var collection = db.get('user');

	var uName = req.body.oprusername;
	var uPass = req.body.oprpassword;

	var query = collection.find({'username':uName}).then(
		function(value)
		{
			if(value[0])
			{
				if (validatePassword(uPass, value[0].password))
				{
					req.session.user = value[0];
					req.session.user.password = "";
					res.redirect("/");
				}
				else
				{
					res.redirect("/");
				}
			}
			else
			{
				res.redirect("/");
			}

		}
	);
});

function sortDates(arr)
{
  var n = arr.length;
  for (i = 1; i < n; i++)
  {
    for (j = 0; j < (n-i); j++)
    {
      if (arr[j].date < arr[j+1].date)
      {
        var tempArray = arr[j];
        arr[j] = arr[j+1];
        arr[j+1] = tempArray;
      }
    }
  }
}

var saltLength = 10;

var generateSalt = function()
{
	var set = '0123456789abcdefghijklmnopqurstuvwxyzABCDEFGHIJKLMNOPQURSTUVWXYZ';
	var salt = '';
	for (var i = 0; i < saltLength; i++) {
		var p = Math.floor(Math.random() * set.length);
		salt += set[p];
	}
	return salt;
};

var hash = function(str)
{
	return crypto.createHash('sha256').update(str).digest('HEX');
};

var generateID = function()
{
  return crypto.createHash('sha256').update(generateSalt()).digest('HEX');
};

var saltAndHash = function(pass)
{
	var salt = generateSalt();
	return salt + hash(pass + salt) + hash(salt);
};

var validatePassword = function(plainPass, hashedPass)
{
	var salt = hashedPass.substr(0, saltLength);
	var validHash = salt + hash(plainPass + salt) + hash(salt);
	if (hashedPass === validHash)
  {return true;}
	else
  {return false;}
};

Array.prototype.contains = function(obj) {
    var i = this.length;
    while (i--) {
        if (this[i] === obj) {
            return true;
        }
    }
    return false;
};

module.exports = router;
