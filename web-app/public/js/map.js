document.addEventListener('DOMContentLoaded', () => {
    // Connect to WebSocket
    const ws = new WebSocket(wsUrl);

    // Initialize map
    const map = L.map('map').setView([20, 0], 2);

    // Tile layer (using OpenStreetMap by default)
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png').addTo(map);

    // Store device markers
    const deviceMarkers = {};
    const infoPanel = document.getElementById('device-info');

    const dummyDevices = [
        {
            device_id: "TAG_001",
            latitude: -1.2921 + (0.1 * Math.random()),  // Nairobi center ± ~11km
            longitude: 36.8219 + (0.1 * Math.random()) // ~11km at equator
        },
        {
            device_id: "TAG_002",
            latitude: -1.2921 + (0.15 * (Math.random() - 0.5)),
            longitude: 36.8219 + (0.15 * (Math.random() - 0.5))
        },
        {
            device_id: "TAG_003",
            latitude: -1.35 + (0.2 * Math.random()),
            longitude: 36.75 + (0.2 * (Math.random() - 0.3))
        },
        {
            device_id: "TAG_004",
            latitude: -1.20 + (0.18 * (Math.random() - 0.4)),
            longitude: 36.90 + (0.1 * Math.random())
        },
        {
            device_id: "TAG_005",
            latitude: -1.40 + (0.25 * Math.random()),
            longitude: 36.70 + (0.25 * Math.random())
        }
    ]

    const warningZones = [
        {
            name: "Critical Zone",
            center: [-1.2921, 36.8219], // Nairobi coordinates
            radius: 5000, // 5km in meters
            color: '#ff0000',
            fillOpacity: 0.2,
            weight: 2
        },
        {
            name: "High Alert Zone",
            center: [-1.2921, 36.8219],
            radius: 15000, // 15km
            color: '#ff6600',
            fillOpacity: 0.15,
            weight: 2
        },
        {
            name: "Watch Zone",
            center: [-1.2921, 36.8219],
            radius: 40000, // 40km
            color: '#ffcc00',
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

    // Plot initial locations
    if (initialLocations && initialLocations.length > 0) {
        // initialLocations.forEach(location => {
        //     updateOrCreateMarker(location);
        // });

        dummyDevices.forEach(location => {
            updateOrCreateMarker(location);
        });

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
            console.log("creating new marker")
            // Create new marker
            deviceMarkers[device_id] = L.marker([latitude, longitude], {
                title: device_id,
                icon: L.divIcon({ className: 'device-marker' })
            }).addTo(map);

            // Add popup
            deviceMarkers[device_id].bindPopup(generatePopupContent(location));

            // Add click handler to show info in panel
            deviceMarkers[device_id].on('click', () => {
                showDeviceInfo(location);
            });
        }

    }

    // Generate popup content
    function generatePopupContent(location) {
        return `
      <b>Device:</b> ${location.deviceId}<br>
      <b>Location:</b> ${location.latitude.toFixed(6)}, ${location.longitude.toFixed(6)}<br>
      <b>Last update:</b> ${new Date(location.timestamp).toLocaleString()}<br>
      ${location.additionalData ? `
        <b>Battery:</b> ${location.additionalData.battery || 'N/A'}%<br>
        <b>Temperature:</b> ${location.additionalData.temperature || 'N/A'}°C
      ` : ''}
    `;
    }

    // Show device info in panel
    function showDeviceInfo(location) {
        infoPanel.innerHTML = `
      <h4>${location.device_id}</h4>
      <p><strong>Coordinates:</strong><br>
      ${location.latitude.toFixed(6)}, ${location.longitude.toFixed(6)}</p>
      <p><strong>Last Update:</strong><br>
      ${new Date(location.timestamp).toLocaleString()}</p>
      ${location.additionalData ? `
        <p><strong>Additional Data:</strong></p>
        <ul>
          ${location.additionalData.battery ? `<li>Battery: ${location.additionalData.battery}%</li>` : ''}
          ${location.additionalData.temperature ? `<li>Temperature: ${location.additionalData.temperature}°C</li>` : ''}
          <!-- Add more fields as needed -->
        </ul>
      ` : ''}
    `;
    }
});

const center =  [-1.2921, 36.8219];

function isWithinRadius(lat, lng, centerLat, centerLng, maxKm) {
    const R = 6371; // Earth's radius in km
    const dLat = (lat - centerLat) * Math.PI / 180;
    const dLng = (lng - centerLng) * Math.PI / 180;
    const a =
        Math.sin(dLat/2) * Math.sin(dLat/2) +
        Math.cos(centerLat * Math.PI / 180) *
        Math.cos(lat * Math.PI / 180) *
        Math.sin(dLng/2) * Math.sin(dLng/2);
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
    return R * c <= maxKm;
}

// Geofencing zones
function createGeofencingZones(locations) {
    const zones = {
        safe: { color: 'green', radius: 5 },
        warning: { color: 'orange', radius: 10 },
        danger: { color: 'red', radius: 20 } // in KM
    };

    locations.forEach(location => {
        L.circle([location.latitude, location.longitude], {
            color: zones[location.status].color,
            radius: zones[location.status].radius
        }).addTo(map);
    });
}