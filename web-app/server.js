const express = require('express')
const mongoose = require('mongoose')
const path = require('path');
const WebSocket = require('ws');

//const dbURL = "mongodb+srv://emwiti658:nU3mmvXQH1OA7BVr@ear-tag-cluster.qq2lahi.mongodb.net/?retryWrites=true&w=majority&appName=ear-tag-cluster"
const dbURL = "mongodb+srv://emwiti658:nU3mmvXQH1OA7BVr@ear-tag-cluster.qq2lahi.mongodb.net/?appName=ear-tag-cluster";

const app= express()

app.set('view engine', 'ejs')
app.set('views', path.join(__dirname, 'views'))

//middleware
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));


// start the server
const server = app.listen(3000,'0.0.0.0', () => {
    console.log('server started on port 3000')
})

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
    acc_mag: Number,
    timestamp: {type: Date, default: Date.now}
})

// create model
const LocationData = mongoose.model('locationData', locationSchema);

// create a websockets server
const wss = new WebSocket.Server({server});

// watch for location change
const locationChangeStream = LocationData.watch();

wss.on('connection', (ws) => {
    console.log('New Client connected');

    // send initial location
    LocationData.find().sort({timestamp:-1}).limit(10)
        .then(locations => {
            ws.send(JSON.stringify({
                type: 'initial',
                data: locations
            }))
        })

    // listen for change and broadcast to all clients
    locationChangeStream.on('change', (change) => {
        if (change.operationType === 'insert') {
            console.log("New item logged")
            LocationData.findById(change.documentKey._id)
                .then(newLocation => {
                    wss.clients.forEach(client => {
                        if (client.readyState === WebSocket.OPEN) {
                            client.send(JSON.stringify({
                                type: 'update',
                                data: newLocation
                            }));
                        }
                    });
                });
        }
    })

    ws.on('close', () => {
        console.log('Client disconnected');
    });
})
// API endpoint to get latest locations (for web interface)
app.get('/', async (req, res) => {
    try {
        const locations = await LocationData.find().sort({ timestamp: -1 }).limit(10);
        //res.json(locations);
        res.render('index', {
            title: "WildPing",
            initialLocations: JSON.stringify(locations),
            mapboxAccessToken: process.env.MAPBOX_ACCESS_TOKEN || ''
        })

    } catch (err) {
        console.error(err);
        res.status(500).send('Server Error');
    }
});



app.post('/api/location', async (req,res) => {
  try {
    const {device_id, latitude, longitude, acc_mag} = req.body;

    const new_location = new LocationData({
        device_id,
        latitude,
        longitude,
        acc_mag
    })

    const savedLocation = await new_location.save();
    res.status(201).json(savedLocation)

  } catch (error) {
    res.status(400).json({ message: error.message });
  }
})

// API Endpoint for Locations
app.get('/api/locations', async (req, res) => {

    try {
        const locations = await Location.find().sort({ timestamp: -1 });
        res.json(locations);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

