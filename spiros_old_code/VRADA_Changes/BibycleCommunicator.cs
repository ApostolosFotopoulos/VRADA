using UnityEngine;

public class BicycleCommunicator : MonoBehaviour
{
    private UDPBroadcastListen udpListener;
    private BicycleDetails bicycleDetails;

    private int nextUpdate = 1;
    void Start()
    {
        this.udpListener = new UDPBroadcastListen();
        this.bicycleDetails = new BicycleDetails(0.0F, 0.0F, 0, 0.0F, 0.0F, 0, 0);
    }
    void Update()
    {
        if (Time.time >= nextUpdate)
        {
            nextUpdate = Mathf.FloorToInt(Time.time) + 1;
            this.bicycleDetails = udpListener.Update(bicycleDetails);
        }

    }

    public BicycleDetails GetDetails()
    {
        return bicycleDetails;
    }

    public void ChangeLevel(int level){
        udpListener.ChangeLevel(level);
    }
}