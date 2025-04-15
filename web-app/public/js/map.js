document.addEventListener('DOMContentLoaded', () => {
    // Initialize map
    const map = L.map('map').setView([20, 0], 2);

    // Tile layer (using OpenStreetMap by default)
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);

    // Store device markers
    const deviceMarkers = {};
    const infoPanel = document.getElementById('device-info');

    // Connect to WebSocket
    const ws = new WebSocket(wsUrl);

    // Plot initial locations
    if (initialLocations && initialLocations.length > 0) {
        initialLocations.forEach(location => {
            updateOrCreateMarker(location);
        });

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
            console.log(locations)

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
        const { deviceId, coordinates, additionalData } = location;

        if (deviceMarkers[deviceId]) {
            // Update existing marker
            deviceMarkers[deviceId].setLatLng([coordinates.lat, coordinates.lng]);
            deviceMarkers[deviceId].setPopupContent(generatePopupContent(location));
        } else {
            // Create new marker
            deviceMarkers[deviceId] = L.marker([coordinates.lat, coordinates.lng], {
                title: deviceId,
                icon: L.divIcon({ className: 'device-marker' })
            }).addTo(map);

            // Add popup
            deviceMarkers[deviceId].bindPopup(generatePopupContent(location));

            // Add click handler to show info in panel
            deviceMarkers[deviceId].on('click', () => {
                showDeviceInfo(location);
            });
        }
    }

    // Generate popup content
    function generatePopupContent(location) {
        return `
      <b>Device:</b> ${location.deviceId}<br>
      <b>Location:</b> ${location.coordinates.lat.toFixed(6)}, ${location.coordinates.lng.toFixed(6)}<br>
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
      <h4>${location.deviceId}</h4>
      <p><strong>Coordinates:</strong><br>
      ${location.coordinates.lat.toFixed(6)}, ${location.coordinates.lng.toFixed(6)}</p>
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