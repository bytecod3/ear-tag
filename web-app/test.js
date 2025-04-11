// API Endpoints
app.post('/api/sensor-data', async (req, res) => {
    try {
        const { deviceId, sensorType, value, unit } = req.body;
        
        const newData = new SensorData({
            deviceId,
            sensorType,
            value,
            unit
        });

        const savedData = await newData.save();
        res.status(201).json(savedData);
    } catch (error) {
        res.status(400).json({ message: error.message });
    }
});