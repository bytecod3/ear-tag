document.addEventListener('DOMContentLoaded', () => {
    // Connect to WebSocket
    const ws = new WebSocket(wsUrl);
    const center = [-1.2921, 36.8219]
    // Initialize map
    const map = L.map('map').setView(center, 5);

    // Tile layer (using OpenStreetMap by default)
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png').addTo(map);

    // center marker
    // Create new marker
    const mark = L.marker(center, {
        title: "Center",
        icon: L.divIcon({ className: 'center-marker' })
    }).addTo(map).bindPopup()

    // Store device markers
    const deviceMarkers = {};
    const infoPanel = document.getElementById('device-info');

    const dummyDevices = [
        {
            device_id: "TAG_001",
            latitude: -1.2921 + (0.1 * Math.random()),
            longitude: 36.8219 + (0.1 * Math.random()),
            timestamp: "2025-04-15T15:03:18.567+00:00",
            acc_mag: 5.6

        },
        {
            device_id: "TAG_002",
            latitude: -1.2921 + (0.15 * (Math.random() - 0.5)),
            longitude: 36.8219 + (0.15 * (Math.random() - 0.5)),
            timestamp: "2025-04-15T15:03:18.567+00:00",
            acc_mag: 6
        },
        {
            device_id: "TAG_003",
            latitude: -1.35 + (0.2 * Math.random()),
            longitude: 36.75 + (0.2 * (Math.random() - 0.3)),
            timestamp: "2025-04-15T15:03:18.567+00:00",
            acc_mag: 2
        },
        {
            device_id: "TAG_004",
            latitude: -1.20 + (0.18 * (Math.random() - 0.4)),
            longitude: 36.90 + (0.1 * Math.random()),
            timestamp: "2025-04-15T15:03:18.567+00:00",
            acc_mag: 10
        },
        {
            device_id: "TAG_005",
            latitude: -1.40 + (0.25 * Math.random()),
            longitude: 36.70 + (0.25 * Math.random()),
            timestamp: "2025-04-15T15:03:18.567+00:00",
            acc_mag: 4
        }
    ]

    const warningZones = [
        {
            name: "Safe Zone",
            center: [-1.2921, 36.8219], // Nairobi coordinates
            radius: 5000, // 5km in meters
            color: '#4CAF50',
            fillOpacity: 0.2,
            weight: 2
        },
        {
            name: "Warning Zone",
            center: [-1.2921, 36.8219],
            radius: 15000, // 15km
            color: '#FFC107',
            fillOpacity: 0.15,
            weight: 2
        },
        {
            name: "Critical Zone",
            center: [-1.2921, 36.8219],
            radius: 20000, // 20km
            color: '#ff0000',
            fillOpacity: 0.1,
            weight: 2
        }
    ];

    // 2. Create the circles on your map
    warningZones.forEach(zone => {
        L.circle(zone.center, {
            radius: zone.radius,
            color: zone.color,
            fillColor: zone.color,
            fillOpacity: zone.fillOpacity,
            weight: zone.weight
        })
            .bindPopup(`<b>${zone.name}</b><br>Radius: ${zone.radius/1000}km`)
            .addTo(map);
    });

    // 3. Add a legend (optional but recommended)
    const legend = L.control({position: 'bottomright'});
    legend.onAdd = function(map) {
        const div = L.DomUtil.create('div', 'legend');
        div.innerHTML = `
        <h4>Warning Zones</h4>
        <div><span style="background: #ff0000; "></span> Critical (>20km)</div>
        <div><span style="background: #ff6600; opacity: 0.15"></span> High Alert (15km)</div>
        <div><span style="background: #5ced73; opacity: 0.1"></span> Watch (<5km) </div>
    `;
        return div;
    };
    legend.addTo(map);

    // Plot initial locations
    if (initialLocations && initialLocations.length > 0) {
        initialLocations.forEach(location => {
            updateOrCreateMarker(location);
        });

        // to simulate turn this on
        // dummyDevices.forEach(location => {
        //     updateOrCreateMarker(location);
        // });

        createGeofencingZones(dummyDevices);

        // Fit map to markers
        const markers = Object.values(deviceMarkers);
        if (markers.length > 0) {
            const group = new L.featureGroup(markers);
            map.fitBounds(group.getBounds());
        }

    }

    // WebSocket handlers
    ws.onmessage = (event) => {
        const message = JSON.parse(event.data);

        if (message.type === 'update') {
            updateOrCreateMarker(message.data);
        }
    };

    ws.onerror = (error) => {
        console.error('WebSocket error:', error);
    };

    // Refresh button
    document.getElementById('refresh-btn').addEventListener('click', async () => {
        console.log("Refresh clicked")
        try {
            const response = await fetch('/api/locations');
            const locations = await response.json();

            // Clear existing markers
            Object.values(deviceMarkers).forEach(marker => map.removeLayer(marker));
            Object.keys(deviceMarkers).forEach(key => delete deviceMarkers[key]);

            // Add new markers
            locations.forEach(location => {
                updateOrCreateMarker(location);
            });

            // Fit map to markers
            const markers = Object.values(deviceMarkers);
            if (markers.length > 0) {
                const group = new L.featureGroup(markers);
                map.fitBounds(group.getBounds());
            }
        } catch (err) {
            console.error('Error refreshing locations:', err);
        }
    });

    // Helper function to update or create marker
    function updateOrCreateMarker(location) {
        const { device_id, latitude, longitude, additionalData } = location;

        console.log(location)

        if (deviceMarkers[device_id]) {
            // Update existing marker
            deviceMarkers[device_id].setLatLng([latitude, longitude]);
            deviceMarkers[device_id].setPopupContent(generatePopupContent(location));
        } else {

            // GEO-FENCING - label according to proximity to center
            let d = distFromCenter(latitude,longitude, center[0], center[1])
            console.log(d)
            let zone = 'critical'
            if(d <= 5) {
                // Create new marker
                zone = "safe"
            } else if(d <= 15){
                zone = "warning"
            } else {
                console.log("Ths is criticl")
                zone = "critical"
            }

            deviceMarkers[device_id] = L.marker([latitude, longitude], {
                title: device_id,
                icon: L.divIcon({ className: 'device-marker-'+zone })
            }).addTo(map);

            // Add popup
            deviceMarkers[device_id].bindPopup(generatePopupContent(location));

            // Add click handler to show info in panel
            deviceMarkers[device_id].on('click', () => {
                showDeviceInfo(location, zone);
            });
        }

    }

    // Generate popup content
    function generatePopupContent(location) {
        return `
      <b>Device:</b> ${location.device_id}<br>
      <b>Location:</b> ${location.latitude.toFixed(6)}, ${location.longitude.toFixed(6)}<br>
      <b>Last update:</b> ${new Date(location.timestamp).toLocaleString()}<br>
      ${location.additionalData ? `
        <b>Battery:</b> ${location.additionalData.battery || 'N/A'}%<br>
        <b>Temperature:</b> ${location.additionalData.temperature || 'N/A'}°C
      ` : ''}
    `;
    }

    // Show device info in panel
    function showDeviceInfo(location, zone) {

        // check shock levels
        let level = ''
        let phy_shock =''

        if(location.acc_mag > 3) {
            level = 'shock'
            phy_shock = 'SHOCK'
        } else {
            level = 'safe'
            phy_shock = 'NOMINAL'
        }
        let geo_class = 'acc-' + level

        // check geographical areas
        let geo_level = 'geo-'+zone


        infoPanel.innerHTML = `
      <h4>Device ID:${location.device_id}</h4>
      
      <p><strong>Coordinates:</strong><br>
      ${location.latitude.toFixed(6)}, ${location.longitude.toFixed(6)}</p>
      
      <p><strong>Resultant acceleration:</strong><br>
      <span class = "${geo_class}">${location.acc_mag} </span>
      </p>
      
      <p><strong>Physical shock condition:</strong><br>
      <span class = "${geo_class}">${phy_shock} </span>
      </p>
      
      <p><strong>Geographical safety level:</strong><br>
 
      <span class = ${geo_level} >${zone} </span>
      </p>
    
      
      <p><strong>Last Update:</strong><br>
      ${new Date(location.timestamp).toLocaleString()}</p>
      ${location.additionalData ? `
      ` : ''}
    `;
    }
});

const center =  [-1.2921, 36.8219];

function distFromCenter(lat, lng, centerLat, centerLng) {
    const R = 6371; // Earth's radius in km
    const dLat = (lat - centerLat) * Math.PI / 180;
    const dLng = (lng - centerLng) * Math.PI / 180;
    const a =
        Math.sin(dLat/2) * Math.sin(dLat/2) +
        Math.cos(centerLat * Math.PI / 180) *
        Math.cos(lat * Math.PI / 180) *
        Math.sin(dLng/2) * Math.sin(dLng/2);
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
    return R * c
}

// Geofencing zones
function createGeofencingZones(locations) {
    const zones = {
        safe: { color: 'green', radius: 5 },
        warning: { color: 'orange', radius: 10 },
        danger: { color: 'red', radius: 20 } // in KM
    };

    // locations.forEach(location => {
    //     L.circle([location.latitude, location.longitude], {
    //         color: zones[location.status].color,
    //         radius: zones[location.status].radius
    //     }).addTo(map);
    // });
}