const express = require('express')
const ejs = require('ejs')
const mongoose = require('mongoose')
const bodyParser = require('body-parser');
const Event = require('./models/event')
const path = require('path');

//const dbURL = "mongodb+srv://emwiti658:nU3mmvXQH1OA7BVr@ear-tag-cluster.qq2lahi.mongodb.net/?retryWrites=true&w=majority&appName=ear-tag-cluster"
const dbURL = "mongodb+srv://emwiti658:nU3mmvXQH1OA7BVr@ear-tag-cluster.qq2lahi.mongodb.net/?appName=ear-tag-cluster";

const app= express()
app.set('view engine', 'ejs')
app.use(bodyParser.json());
app.set('views', path.join(__dirname, 'views'))

// server sttic files from public directory 
app.use(express.static(path.join(__dirname, 'public')))

// connect to database
//databse connection
mongoose
  .connect(dbURL)
  .then((result) => {
    console.log('Connected to MongoDB');
    app.listen(3000, () => {
      console.log('Server started on port 3000');
    });
  })
  .catch((err) => {
    console.error('Could not connect to MongoDB:', err);
  });


/*
Schema
TODO: move this to models 
*/
// location schema 
const locationSchema = new mongoose.Schema({
  device_id: String,
  latitude: Number,
  longitude: Number,
  timestamp: {type: Date, default: Date.now}
})
const Location = mongoose.model('Location', locationSchema);

// routing paths

// render the map page
app.get('/', (req, res) => {
    // static location 
    const location = {
        lat: -1.1018,
        lng: 37.0144,
        title: "Ear Tag Monitor"
    }

    res.render('map', {location})
})

// create model
const locationData = mongoose.model('locationData', locationSchema);


app.post('/api/location', async (req,res) => {
  try {
    const {device_id, latitude, longitude} = req.body;

    const new_location = new locationData({
        device_id,
        latitude,
        longitude
    })

    const savedLocation = await new_location.save();
    res.status(201).json(savedLocation)

  } catch (error) {
    res.status(400).json({ message: error.message });
  }
})

// fetch data stored on database 
app.get('/api/location', async (req, res) => {
  try {
      const data = await locationData.find().sort({ timestamp: -1 });
      res.json(data);
  } catch (error) {
      res.status(500).json({ message: error.message });
  }
});

// start the server
app.listen(3000,'0.0.0.0', () => {
    console.log('server started on port 3000')
})