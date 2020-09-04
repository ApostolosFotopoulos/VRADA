using UnityEngine;

namespace PathCreation.Examples
{
    // Moves along a path at constant speed.
    // Depending on the end of path instruction, will either loop, reverse, or stop at the end of the path.
    public class PathFollower : MonoBehaviour
    {
        float TimeInterval;
        public PathCreator pathCreator;
        public EndOfPathInstruction endOfPathInstruction;
        public float speed = 0;
        float distanceTravelled;

        private BicycleCommunicator bicycleCommunicator;

        void Start() {
            if (pathCreator != null)
            {
                // Subscribed to the pathUpdated event so that we're notified if the path changes during the game
                pathCreator.pathUpdated += OnPathChanged;
            }

            if (this.gameObject.GetComponent<Requester>() == null)
            {
                this.gameObject.AddComponent<Requester>();
            }
            if(this.gameObject.GetComponent<BicycleCommunicator>() == null){
                this.gameObject.AddComponent<BicycleCommunicator>();
            }
            if(!this.gameObject.GetComponent<BicycleCommunicator>().enabled){
                this.gameObject.GetComponent<BicycleCommunicator>().enabled = true;
            }
            bicycleCommunicator = this.gameObject.GetComponent<BicycleCommunicator>();
            fetchBikeSpeed();

        }

        void Update()
        {
            if (pathCreator != null)
            {
                distanceTravelled += speed * Time.deltaTime;
                transform.position = pathCreator.path.GetPointAtDistance(distanceTravelled, endOfPathInstruction);
                transform.rotation = pathCreator.path.GetRotationAtDistance(distanceTravelled, endOfPathInstruction);
                transform.eulerAngles += new Vector3(0.0f, 180.0f, 0.0f);
            }

            // one in seconds fetch speed
            TimeInterval += Time.deltaTime;
            if (TimeInterval >= 1)
            {
                TimeInterval = 0;
                fetchBikeSpeed();
            }

        }

        // If the path changes during the game, update the distance travelled so that the follower's position on the new path
        // is as close as possible to its position on the old path
        void OnPathChanged() {
            distanceTravelled = pathCreator.path.GetClosestDistanceAlongPath(transform.position);
        }

        private void fetchBikeSpeed()
        {
            // we now read the speed from the listener which updates the speed value by listening to ESP broadcast packets 
            this.speed = bicycleCommunicator.GetDetails().Speed;
        }

    }
}