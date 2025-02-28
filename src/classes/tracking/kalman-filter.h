// Class to smoothen blendshape value changes
class KalmanFilter {
public:
	KalmanFilter(float processNoise = 0.01f, float measurementNoise = 0.1f, float estimatedError = 1.0f,
		     float initialValue = 0.0f)
		: q(processNoise),
		  r(measurementNoise),
		  p(estimatedError),
		  x(initialValue)
	{
	}

	float update(float measurement)
	{
		// Prediction Update
		p = p + q; // Increase the error estimate

		// Measurement Update
		float k = p / (p + r);         // Kalman gain
		x = x + k * (measurement - x); // Update estimate with measurement
		p = (1 - k) * p;               // Update error covariance

		return x; // Return the smoothed value
	}

private:
	float q; // Process noise covariance
	float r; // Measurement noise covariance
	float p; // Estimated error covariance
	float x; // State estimate
};
