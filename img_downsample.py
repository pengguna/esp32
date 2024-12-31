from PIL import Image
import argparse 
import time
import paho.mqtt.client as mqtt
import json
import requests
import os

import spotipy
from spotipy.oauth2 import SpotifyOAuth

def yoink_spotify(sp):

    current_track = sp.current_playback()

    album_art_url = current_track['item']['album']['images'][0]['url']

    return current_track['item']['id'], album_art_url


def process_and_publish_image(input_path, mqtt_broker, mqtt_topic, mqtt_user, mqtt_pass):
    """
    Process an image, extract RGB values, and publish to an MQTT topic.

    :param input_path: Path to the input JPG file.
    :param mqtt_broker: MQTT broker address.
    :param mqtt_topic: MQTT topic to publish the RGB data.
    """
    try:
        # Step 1: Open the input image
        with Image.open(input_path) as img:
            # Step 2: Resize the image to 8x8
            img_resized = img.resize((8, 8), Image.Resampling.LANCZOS)

            # Step 3: Extract RGB values
            img_rgb = img_resized.convert("RGB")  # Ensure the image is in RGB mode
            pixels = list(img_rgb.getdata())     # Get RGB values for all pixels
            flat_px = [int(v*0.1) for p in pixels for v in p]

        # Step 4: Publish to MQTT
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        client.username_pw_set(mqtt_user, mqtt_pass);
        client.connect(mqtt_broker)

        data_to_dump = json.dumps(flat_px)[1:-1]
        data_to_dump = "".join(data_to_dump.split())
        print('dumping', data_to_dump)
        # data_to_dump="1,0,0"
        client.publish(mqtt_topic, data_to_dump)
        print(f"Published RGB data to MQTT topic '{mqtt_topic}' on broker '{mqtt_broker}'")
        client.disconnect()

    except Exception as e:
        print(f"An error occurred: {e}")

def main():
    parser = argparse.ArgumentParser(description="Downsample an image and publish RGB data to MQTT.")
    parser.add_argument("--input_file", default='input.jpg', help="Path to the input image file (JPG format).")
    parser.add_argument("--user", help="MQTT username.")
    parser.add_argument("--pwd", help="MQTT password.")
    parser.add_argument("--broker", help="MQTT broker address.")
    parser.add_argument("--topic", default="/topic/qos0", help="MQTT topic to publish RGB data (default: 'image/rgb').")

    args = parser.parse_args()

    with open("secrets.json", 'r') as f:
        secrets = json.load(f)

    sp = spotipy.Spotify(auth_manager=SpotifyOAuth(
        client_id=secrets['client_id'], 
        client_secret=secrets['client_secret'], 
        redirect_uri=secrets['redirect_uri'], 
        scope="user-library-read user-read-playback-state"))


    last_render = ''

    while True:
        print('tick')
        img_id, img_url = yoink_spotify(sp)

        img_paf = img_id + '.jpg'
        if os.path.exists(img_paf):
            print('already exists!')
        else:
            print('gotta yoink')
            response = requests.get(img_url)
            assert response.status_code == 200
            with open(img_paf, 'wb') as f:
                f.write(response.content)

        if img_paf != last_render:
            print('need to rerender')
            last_render = img_paf

            process_and_publish_image(
                input_path=img_paf,
                mqtt_broker=args.broker,
                mqtt_topic=args.topic,
                mqtt_user=args.user,
                mqtt_pass=args.pwd
            )

        time.sleep(10)
    

if __name__ == "__main__":
    main()
