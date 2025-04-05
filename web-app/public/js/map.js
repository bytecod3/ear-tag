document.addEventListener('DOMContentLoaded', function() {
    // Initialize the map
    const map = L.map('map').setView([locationData.lat, locationData.lng], 13);

    // Add OpenStreetMap tiles
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);

    // Add a marker for the location
    L.marker([locationData.lat, locationData.lng])
        .addTo(map)
        .bindPopup(`<b>${locationData.title}</b><br>Lat: ${locationData.lat}<br>Lng: ${locationData.lng}`)
        .openPopup();
});