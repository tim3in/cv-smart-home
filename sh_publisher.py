import cv2
import time
import paho.mqtt.client as mqtt
from inference_sdk import InferenceHTTPClient

broker_address = "broker.hivemq.com"
port = 1883

client = mqtt.Client(client_id="smart_home_1")
client.connect(broker_address, port)

# Initialize InferenceHTTPClient
CLIENT = InferenceHTTPClient(
    api_url="https://detect.roboflow.com",
    api_key="YOUR_ROBOFLOW_API_KEY"
)

video = cv2.VideoCapture(0)

while True:
    ret, frame = video.read()
    if not ret:
        break

    # Infer on the frame
    result = CLIENT.infer(frame, model_id="home-monitoring/1")
    detections = result['predictions']

    for bounding_box in detections:
        x0 = int(bounding_box['x'] - bounding_box['width'] / 2)
        x1 = int(bounding_box['x'] + bounding_box['width'] / 2)
        y0 = int(bounding_box['y'] - bounding_box['height'] / 2)
        y1 = int(bounding_box['y'] + bounding_box['height'] / 2)
        class_name = bounding_box['class']
        confidence = bounding_box['confidence']

        # Publish detected class to appropriate MQTT topic
        if class_name in ["light1_on", "light1_off"]:
            client.publish("home/light", class_name)
        elif class_name in ["fan_on", "fan_off"]:
            client.publish("home/fan", class_name)

        cv2.rectangle(frame, (x0, y0), (x1, y1), color=(0, 0, 255), thickness=1)
        cv2.putText(frame, f"{class_name} - {confidence:.2f}", (x0, y0 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5,
                    (0, 0, 255), 1)

    cv2.imshow('frame', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

video.release()
cv2.destroyAllWindows()
