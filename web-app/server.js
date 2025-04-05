const express = require('express')
const ejs = require('ejs')
const mongoose = require('mongoose')
const Event = require('./models/event')
const path = require('path');

const dbURL = "mongodb+srv://emwiti658:nU3mmvXQH1OA7BVr@ear-tag-cluster.qq2lahi.mongodb.net/?retryWrites=true&w=majority&appName=ear-tag-cluster"

const app= express()
app.set('view engine', 'ejs')
app.set('views', path.join(__dirname, 'views'))

// server sttic files from public directory 
app.use(express.static(path.join(__dirname, 'public')))

// routing path 

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

// databse connection
// mongoose
//   .connect(dbURL)
//   .then((result) => {
//     console.log('Connected to MongoDB');
//     app.listen(3000, () => {
//       console.log('Server started on port 3000');
//     });
//   })
//   .catch((err) => {
//     console.error('Could not connect to MongoDB:', err);
//   });


// start the server
app.listen(3000, () => {
    console.log('server started on port 3000')
})