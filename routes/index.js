var express = require('express');
var crypto = require('crypto');
var mongo = require('mongodb');
var session = require('client-sessions');
var numeral = require('numeral');
var JSZip = require("jszip");
var router = express.Router();
var y = [
	2013,
	2014,
	2015,
	2016,
	2017
];
/* GET home page. */
router.get('/', function (req, res, next)
{
  validateUser(req, res, function ()
  {
    res.render('index',
    {
      title: 'Home',
      years: y,
      a: adminPrivledge(req.session.user)
    });
  }, function ()
  {
    res.render('login',
    {
      title: 'Login'
    });
  });
});
router.get('/users', function (req, res, next)
{
  validateUser(req, res, function ()
  {
		if(adminPrivledge(req.session.user))
		{
			var db = req.dbUser;
      var collection = db.get('user');
      collection.find(
      {}).then(function (value)
      {
        res.render('users',
        {
          title: 'Users',
          users: value
        });
      });
		}
    else
    {
      res.redirect('/');
    }
  }, function ()
  {
    res.redirect('/');
  });
});

router.get('/r', function (req, res, next)
{
  res.redirect('/');
});

router.post('/r/:year/:id/upload', function (req, res, next)
{
  var year = req.params.year;
  var repoID = req.params.id;
  var data = req.body.data;
  var name = req.body.name;
  validateUser(req, res, function ()
  {
		if(superPrivledge(req.session.user))
		{
			if (y.contains(parseInt(year)))
	    {
	      var dbD = req.dbData;
	      var col = "OPR" + year;
	      var yearData = dbD.get(col);
	      var newID = generateID();
	      yearData.update(
	      {
	        _id: repoID
	      },
	      {
	        $addToSet:
	        {
	          files:
	          {
	            "id": newID,
	            "name": name,
	            "data": data
	          }
	        }
	      }).then(function ()
	      {
	        res.json(
	        {
	          "response": "OK",
	          "error": "",
	          "file": name
	        });
	      });
	    }
	    else
	    {
	      res.json(
	      {
	        "response": "FAIL",
	        "error": "Year is invalid!",
	        "file": ""
	      });
	    }
		}
		else
		{
			res.json(
	    {
	      "response": "FAIL",
	      "error": "Invalid Session",
	      "file": ""
	    });
		}
  }, function ()
  {
    res.json(
    {
      "response": "FAIL",
      "error": "Invalid Session",
      "file": ""
    });
  });
});
router.get('/r/:year', function (req, res, next)
{
  var year = req.params.year;
  var db = req.dbUser;
  var collection = db.get('user');
	validateUser(req,res,function(){
		if (y.contains(parseInt(year)))
		{
			var dbD = req.dbData;
			var col = "OPR" + year;
			var yearData = dbD.get(col);
			yearData.find(
			{}).then(function (r)
			{
				if (r.length > 0)
				{
					var d = new Date();
					sortDates(r);
					var recent = r[0];
					var zip = new JSZip();
					r = recent;
					for (i = 0; i < r.files.length; i++)
					{
						var fTypeA = r.files[i].name.split(".");
						var fType = fTypeA[fTypeA.length - 1];
						r.files[i].type = fileType(fType);
						zip.file(r.files[i].name, r.files[i].data,
						{
							base64: true
						});
					}
					zip.generateAsync(
					{
						type: "base64"
					}).then(function (Zipbase64)
					{
						var oD = new Date(recent.date);
						var daysInbetween = daysBetween(oD, d);
						Zipbase64 = "data:application/zip;base64," +
							Zipbase64;
						res.render('repo',
						{
							title: year,
							recentUpload: recent,
							date: oD.toDateString(),
							time: oD.toLocaleTimeString(),
							days: daysInbetween,
							downloadZip: Zipbase64
						});
					});
				}
				else
				{
					res.redirect('/');
				}
			});
		}
		else
		{
			res.redirect('/');
		}
	},function(){
		res.redirect('/');
	});
});
var daysBetween = function (date1, date2)
{
  var one_day = 1000 * 60 * 60 * 24;
  var date1_ms = date1.getTime();
  var date2_ms = date2.getTime();
  var difference_ms = date2_ms - date1_ms;
  return Math.round(difference_ms / one_day);
};
router.get('/r/:year/download', function (req, res, next)
{
  var year = req.params.year;
  validateUser(req, res, function ()
  {
    if (y.contains(parseInt(year)))
    {
      var dbD = req.dbData;
      var col = "OPR" + year;
      var yearData = dbD.get(col);
      yearData.find(
      {}).then(function (r)
      {
        if (r.length > 0)
        {
          sortDates(r);
          r = r[0];
          var zip = new JSZip();
          for (i = 0; i < r.files.length; i++)
          {
            zip.file(r.files[i].name, r.files[i].data,
            {
              base64: true
            });
          }
          var fname = col + ".zip";
          zip.generateAsync(
          {
            type: "base64"
          }).then(function (Zipbase64)
          {
            res.json(
            {
              'response': "OK",
              'error': "",
              'random_string': generateSalt(),
              'date': r.date,
              'zipfile':
              {
                'name': fname,
                'data': Zipbase64
              },
            });
          });
        }
        else
        {
          res.json(
          {
            'response': "FAIL",
            'error': "invalid year",
            'zipfile': ""
          });
        }
      });
    }
  }, function ()
  {
    res.json(
    {
      'response': "FAIL",
      'error': "invalid session",
      'zipfile': ""
    });
  });
});
router.get('/r/:year/history', function (req, res, next)
{
  var year = req.params.year;
  var db = req.dbUser;
  var collection = db.get('user');
	validateUser(req,res,function(){
		if (y.contains(parseInt(year)))
		{
			var dbD = req.dbData;
			var col = "OPR" + year;
			var yearData = dbD.get(col);
			yearData.find(
			{}).then(function (r)
			{
				if (r.length > 0)
				{
					sortDates(r);
					for (i = 0; i < r.length; i++)
					{
						var cDate = new Date(r[i].date);
						r[i].date = {
							"d": cDate.toDateString(),
							"t": cDate.toLocaleTimeString()
						};
					}
					var da = false;
					if (req.session.user.ut > 0)
					{
						da = true;
					}
					res.render('repohistory',
					{
						title: year,
						historyList: r,
						delaccess:da
					});
				}
			});
		}
		else
		{
			res.redirect('/');
		}
	},function(){
		res.redirect('/');
	});
});
router.get('/r/:year/:uploadID', function (req, res, next)
{
  var year = req.params.year;
  var uploadID = req.params.uploadID;
  var db = req.dbUser;
  var collection = db.get('user');
	validateUser(req,res,function(){
		if (y.contains(parseInt(year)))
		{
			var dbD = req.dbData;
			var col = "OPR" + year;
			var yearData = dbD.get(col);
			yearData.find(
			{
				_id: uploadID
			}).then(function (r)
			{
				if (r.length > 0)
				{
					var d = new Date();
					var zip = new JSZip();
					r = r[0];
					for (i = 0; i < r.files.length; i++)
					{
						var fTypeA = r.files[i].name.split(".");
						var fType = fTypeA[fTypeA.length - 1];
						r.files[i].type = fileType(fType);
						zip.file(r.files[i].name, r.files[i].data,
						{
							base64: true
						});
					}
					zip.generateAsync(
					{
						type: "base64"
					}).then(function (Zipbase64)
					{
						var oD = new Date(r.date);
						var daysInbetween = daysBetween(oD, d);
						Zipbase64 = "data:application/zip;base64," +
							Zipbase64;
						res.render('repo',
						{
							title: year,
							recentUpload: r,
							date: oD.toDateString(),
							time: oD.toLocaleTimeString(),
							days: daysInbetween,
							downloadZip: Zipbase64
						});
					});
				}
			});
		}
		else
		{
			res.redirect('/');
		}
	},function(){
		res.redirect('/');
	});
});
router.get('/r/:year/:uploadID/:fileID', function (req, res, next)
{
  var year = req.params.year;
  var uploadID = req.params.uploadID;
  var fileID = req.params.fileID;
  var db = req.dbUser;
  var collection = db.get('user');
	validateUser(req,res,function(){
		if (y.contains(parseInt(year)))
		{
			var dbD = req.dbData;
			var col = "OPR" + year;
			var yearData = dbD.get(col);
			yearData.find(
			{
				'_id': uploadID
			},
			{}).then(function (r)
			{
				if (r)
				{
					var fileData;
					for (i = 0; i < r[0].files.length; i++)
					{
						if (r[0].files[i].id === fileID)
						{
							fileData = r[0].files[i];
						}
					}
					if (fileData)
					{
						res.render('fileview',
						{
							title: fileData.name,
							filedata: fileData,
							y: year,
							u: uploadID,
							f: fileID
						});
					}
					else
					{
						res.redirect('/' + year);
					}
				}
				else
				{
					res.redirect('/');
				}
			});
		}
		else
		{
			res.redirect('/');
		}
	},function(){
		res.redirect('/');
	});
});
router.get('/logout', function (req, res)
{
  req.session.reset();
  res.redirect('/');
});
router.post('/r/:year/new', function (req, res)
{
  var year = req.params.year;
  var rdate = req.body.oprdate;
  var rmessage = req.body.oprmessage;
  var db = req.dbUser;
  var collection = db.get('user');
	validateUser(req,res,function(){
		if (superPrivledge(req.session.user))
		{
			if (y.contains(parseInt(year)))
			{
				var dbD = req.dbData;
				var col = "OPR" + year;
				var yearData = dbD.get(col);
				var newRepoFormat = {
					"date": rdate,
					"user": req.session.user.username,
					"message": rmessage,
					"files": []
				};
				yearData.insert(newRepoFormat, function (err, d)
				{
					res.json(
					{
						"response": "OK",
						"error": "",
						"newid": d._id
					});
				});
			}
			else
			{
				res.json(
				{
					"response": "FAIL",
					"error": "invalid year",
					"newid": ""
				});
			}
		}
		else
		{
			res.json(
	    {
	      "response": "FAIL",
	      "error": "invalid user; not authorized",
	      "newid": ""
	    });
		}
	},function(){
		res.json(
    {
      "response": "FAIL",
      "error": "invalid user; not authorized",
      "newid": ""
    });
	});
});
router.get('/remove/:year/:id', function (req, res, next)
{
  var year = req.params.year;
  var repID = req.params.id;
  var db = req.dbUser;
  var collection = db.get('user');
	validateUser(req,res,function(){
		if (superPrivledge(req.session.user))
		{
			if (y.contains(parseInt(year)))
			{
				var dbD = req.dbData;
				var col = "OPR" + year;
				var yearData = dbD.get(col);
				yearData.remove(
				{
					_id: repID
				}).then(function ()
				{
					res.json(
					{
						"response": "OK",
						"error": ""
					});
				});
			}
			else
			{
				res.json(
				{
					"response": "FAIL",
					"error": "invalid year"
				});
			}
		}
		else
		{
			res.json(
	    {
	      "response": "FAIL",
	      "error": "invalid user; not authorized"
	    });
		}
	},function(){
		res.json(
    {
      "response": "FAIL",
      "error": "invalid user; not authorized"
    });
	});
});

router.post('/ulogin', function (req, res)
{
  var db = req.dbUser;
  var collection = db.get('user');
  var uName = req.body.oprusername;
  var uPass = req.body.oprpassword;
  var query = collection.find(
  {
    'username': uName
  }).then(function (value)
  {
    if (value[0])
    {
      if (validatePassword(uPass, value[0].password))
      {
        if (value[0].npc)
        {
          var newst = "/updatepassword/" + value[0]._id.toString() + "/" + hash(value[0]._id.toString()) + "/" + saltAndHash(value[0]._id.toString());
          res.redirect(newst);
        }
        else
        {
          req.session.user = value[0];
					if (parseInt(req.session.user.ut) === 2)
					{
						req.session.user.admin = true;
					}
					else
					{
						req.session.user.admin = false;
					}
          req.session.user.password = "";
          res.redirect("/");
        }
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
  });
});

router.get('/updatepassword/:uid/:rand/:morerand', function(req, res, next){
	var db = req.dbUser;
  var collection = db.get('user');
  var user = req.params.uid;
  collection.find(
  {
    '_id': user
  }).then(function (value)
  {
    if (value)
    {
			res.render('newpass',
			{
				title: "Update Password",
				idu:user
			});
    }
    else
    {
      res.redirect("/users");
    }
  });
});
router.post('/changepassword', function (req, res, next)
{
  var db = req.dbUser;
  var collection = db.get('user');
  var uPass = req.body.password;
	var uid = req.body.uid;
  collection.find(
  {
    '_id': uid
  }).then(function (value)
  {
    if (value)
    {
      collection.update(
				{
					_id:uid
				},
				{
					username:value[0].username,
					password:saltAndHash(uPass),
					npc:false,
					ut:value[0].ut
				}
			).then(function(){
				res.redirect("/");
			});
    }
    else
    {
      res.redirect("/");
    }
  });
});

router.get('/edituser/:uid', function(req, res, next){
	var db = req.dbUser;
  var collection = db.get('user');
  var user = req.params.uid;
	validateUser(req,res,function(){
		if(adminPrivledge(req.session.user))
		{
			collection.find(
		  {
		    '_id': user
		  }).then(function (value)
		  {
		    if (value)
		    {
					res.render('edituser',
					{
						title: "Edit User",
						user:value[0]
					});
		    }
		    else
		    {
		      res.redirect("/users");
		    }
		  });
		}
		else
		{
			res.redirect("/");
		}
	},function(){
		res.redirect("/");
	});
});

router.post('/editusersub', function (req, res, next)
{
  var db = req.dbUser;
  var collection = db.get('user');
	var uid = req.body.uid;
	var username = req.body.username;
	var ut = parseInt(req.body.ut);
	validateUser(req,res,function(){
		if(adminPrivledge(req.session.user))
		{
			collection.find({_id: uid}).then(function(userobj){
				userobj = userobj[0];
				collection.update(
					{
						_id:uid
					},
					{
						username:username,
						password:userobj.password,
						npc:userobj.npc,
						ut:ut
					}
				).then(function(){
					res.redirect("/users");
				});
			});
		}
		else
		{
			res.redirect("/");
		}
	},function(){
		res.redirect("/");
	});
});

router.post('/forcepasswordchange', function (req, res, next)
{
  var db = req.dbUser;
  var collection = db.get('user');
	var uid = req.body.uid;
	validateUser(req,res,function(){
		if(adminPrivledge(req.session.user))
		{
			collection.find({_id: uid}).then(function(userobj){
				userobj = userobj[0];
				collection.update(
					{
						_id:uid
					},
					{
						username:userobj.username,
						password:userobj.password,
						npc:true,
						ut:userobj.ut
					}
				).then(function(){
					res.redirect("/users");
				});
			});
		}
		else
		{
			res.redirect("/");
		}
	},function(){
		res.redirect("/");
	});
});

router.post('/client/login', function (req, res)
{
  var db = req.dbUser;
  var collection = db.get('user');
  var uName = req.body.oprusername;
  var uPass = req.body.oprpassword;
  var query = collection.find(
  {
    'username': uName
  }).then(function (value)
  {
    if (value[0])
    {
      if (validatePassword(uPass, value[0].password))
      {
				if (superPrivledge(value[0]))
				{
					req.session.user = value[0];
	        req.session.user.password = "";
					res.json(
	        {
	          "response": "OK",
	          "error": "",
	          "user": req.session.user.username
	        });
				}
				else
				{
					res.json(
	        {
	          "response": "OK",
	          "error": "",
	          "user": req.session.user.username
	        });
				}

      }
      else
      {
        res.json(
        {
          "response": "FAIL",
          "error": "incorrect username/password"
        });
      }
    }
    else
    {
      res.json(
      {
        "response": "FAIL",
        "cannot find user": ""
      });
    }
  });
});
router.get('/removeuser/:uid', function (req, res, next)
{
  var db = req.dbUser;
  var collection = db.get('user');
  var user = req.params.uid;
	if(adminPrivledge(req.session.user))
	{
		collection.find(
		{
			'_id': user
		}).then(function (value)
		{
			if (value)
			{
				collection.remove(
				{
					_id: user
				}).then(function ()
				{
					res.redirect("/users");
				});
			}
			else
			{
				res.redirect("/users");
			}
		});
	}
	else {
		res.redirect("/");
	}
});
router.post('/add/new/user', function (req, res)
{
  var db = req.dbUser;
  var collection = db.get('user');
  var uName = req.body.username;
  var uPass = req.body.password;
	var ut = parseInt(req.body.ut);
	if (adminPrivledge(req.session.user))
	{
		collection.find(
	  {
	    'username': uName
	  }).then(function (value)
	  {
	    if (value[0])
	    {
	      res.redirect("/users");
	    }
	    else
	    {
	      if (uName.length > 0 && uPass.length > 0)
	      {

	        var userData = {
	          'username': uName,
	          'password': saltAndHash(uPass),
						'ut':ut,
	          'npc': true
	        };
	        collection.insert(userData);
	        res.redirect("/users");
	      }
	      else
	      {
	        res.redirect("/users");
	      }
	    }
	  });
	}
	else
	{
		res.redirect("/");
	}

});
var fileType = function(ft)
{
	var r = "";
	switch (fType)
	{
	case 'txt':
		r = "-text";
		break;
	case 'cpp':
		r = "-code";
		break;
	case 'h':
		r = "-code";
		break;
	case 'c':
		r = "-code";
		break;
	case 'png':
		r = "-image";
		break;
	case 'jpg':
		r = "-image";
		break;
	case 'docx':
		r = "-word";
		break;
	case 'doc':
		r = "-word";
		break;
	case 'xlsx':
		r = "-excel";
		break;
	case 'xls':
		r = "-excel";
		break;
	case 'csv':
		r = "-excel";
		break;
	case 'pdf':
		r = "-pdf";
		break;
	default:
		r = "";
		break;
	}
	return r;
};

var validateUser = function (req, res, callback_success, callback_fail)
{
  var db = req.dbUser;
  var collection = db.get('user');
  if (req.session && req.session.user)
  {
    var query = collection.find(
    {
      'username': req.session.user.username
    }).then(function (value)
    {
      if (value)
      {
					callback_success();
      }
      else
      {
        callback_fail();
      }
    });
  }
  else
  {
    callback_fail();
  }
};

var adminPrivledge = function(user)
{
	if (user)
	{
		if (user.ut === 2)
		{
			return true;
		}
		else {
			return false;
		}
	}
	else
	{
		return false;
	}

};
var superPrivledge = function(user)
{
	if (user)
	{
		if (user.ut > 0)
		{
			return true;
		}
		else {
			return false;
		}
	}
	else
	{
		return false;
	}

};

function sortDates(arr)
{
  var n = arr.length;
  for (i = 1; i < n; i++)
  {
    for (j = 0; j < (n - i); j++)
    {
      if (arr[j].date < arr[j + 1].date)
      {
        var tempArray = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = tempArray;
      }
    }
  }
}
var saltLength = 10;
var generateSalt = function ()
{
  var set =
    '0123456789abcdefghijklmnopqurstuvwxyzABCDEFGHIJKLMNOPQURSTUVWXYZ';
  var salt = '';
  for (var i = 0; i < saltLength; i++)
  {
    var p = Math.floor(Math.random() * set.length);
    salt += set[p];
  }
  return salt;
};
var hash = function (str)
{
  return crypto.createHash('sha256').update(str).digest('HEX');
};
var generateID = function ()
{
  return crypto.createHash('sha256').update(generateSalt()).digest('HEX');
};
var saltAndHash = function (pass)
{
  var salt = generateSalt();
  return salt + hash(pass + salt) + hash(salt);
};
var validatePassword = function (plainPass, hashedPass)
{
  var salt = hashedPass.substr(0, saltLength);
  var validHash = salt + hash(plainPass + salt) + hash(salt);
  if (hashedPass === validHash)
  {
    return true;
  }
  else
  {
    return false;
  }
};
Array.prototype.contains = function (obj)
{
  var i = this.length;
  while (i--)
  {
    if (this[i] === obj)
    {
      return true;
    }
  }
  return false;
};
module.exports = router;
