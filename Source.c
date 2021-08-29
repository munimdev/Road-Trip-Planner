
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <curl/curl.h> //External library in c to send and receive data from websites among other stuff. Downloaded from curl.se/download.html. Installation guide available on bit.ly/37c9Acw
#include <json-c/json.h> //External library for JSON file/data handling in C. Downloaded from github.com/json-c/json-c. Complete installation guide is available on its github page
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

//NOTE! curl is installed on most PC's nowadays days. You can enter "curl example.com" into your cmd(or any terminal) and receive response from that website
//Defines the struct for the memory for data to be received by the program from the server
struct MemoryStruct {
    char* memory; //String to store data in
    size_t size; //Unsigned int to store the number of bytes received
};
//Reference curl.se/libcurl/c/getinmemory.html
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) //Function to download/write all the data the program receives into memory with the parameters required by official libcurl documentation
{
    size_t realsize = size * nmemb; //
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;
    char* ptr = realloc(mem->memory, mem->size + realsize + 1); //Reallocates memory for the string variable as more chunks as downloaded from server
    if (ptr == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize); //Contents is actual data arriving from server
    mem->size += realsize; //Updates the size of memory as an int according to byres received
    mem->memory[mem->size] = 0;

    return realsize; //Returns number of bytes received to curl
}

int main(void)
{
    CURL* curl = curl_easy_init(); //Initalizes curl, as well as returns a curl session handle to the 'curl' variable. This 'curl' variable is used in all curl functions

    struct MemoryStruct chunk; //Declare variable chunk that will store all the server data received
    struct json_object* parsed_json, * route, * distance, * time, * fuel, * info, * info_contents, * message, * m_temp, * statuscode; //Creates different JSON objects to store data into according to the JSON data received by program from the API

    curl_global_init(CURL_GLOBAL_ALL); //Initialize the curl session/handle

    chunk.memory = malloc(1); //Intitializing chunk memory to 1 byte. It will automatically grow as WRITEDATE function is called
    chunk.size = 0; //Setting bytes received to 0 inititially

    char test_url[1000] = "http://www.mapquestapi.com/directions/v2/route?key=JykhZjL9M76QE2YfF6yfkUkxnlOFfZ1W"; //The initial URL that is the Distance Matrix API
    char source_location[100] = "&from="; //Declaring the parameters for the API as strings, which will be added to the URL after user input
    char destination_location[100] = "&to="; //Parameter
    char unit[15] = "&unit="; //Parameter
    char mpg[25] = "&highwayEfficiency="; //Parameter
    char* temp, * temp1, * temp2, * temp3, * temp4, user_input[500] = { "" }; //User input collectively stores all the parameters for the API. This was created just to explain how parameters work
    temp = malloc(90);
    temp1 = malloc(90);
    temp2 = malloc(10);
    temp3 = malloc(20);
    temp4 = malloc(10); //Allocating memory to the temporary strings
    float mileage;
    //Taking inputs for the API parameters
    printf("\nEnter your source location: ");
    fgets(temp, 80, stdin); //Inputting source location
    temp[strlen(temp) - 1] = '\0'; //Removing enter key from input
    strcat(user_input, strcat(source_location, temp)); //Concatanate the input with the parameter, and concatanate parameter to user_input

    printf("\nEnter your destination location: ");
    fgets(temp1, 80, stdin); //Input destination location
    temp1[strlen(temp1) - 1] = '\0'; //Removing enter key from inpit
    strcat(user_input, strcat(destination_location, temp1)); //Concatanate the input with the parameter, and concatanate parameter to user_input

    printf("\nEnter your prefferred distance unit (k or m): ");
    fgets(temp2, 10, stdin); //Input preferred distance unit
    temp2[strlen(temp2) - 1] = '\0'; //Remove enter key from input

    if (strcmp(temp2, "k") == 0) //Checks if k was inputted, and inputs kml value
    {
        printf("\nEnter your vehicle's kml (leave blank to use average value): "); //Because the server only accepts car's fuel efficiency in mpg, we have to convert kml back to mpg and conv
        scanf_s("%f", &mileage);
        mileage *= 2.352;
        sprintf(temp3, "%.2f", mileage); //Converting mileage to a string so it can be concatenated
    }

    else
    {
        printf("\nEnter your vehicle's mpg (leave blank to use average value): ");
        fgets(temp3, 10, stdin);
        temp3[strlen(temp3) - 1] = '\0';
    }

    strcat(user_input, strcat(unit, temp2)); //Concatanate the input with the parameter, and concatanate parameter to user_input
    strcat(user_input, strcat(mpg, temp3));
    strcat(test_url, user_input); //Concatenating all the parameters according to user's liking to the API URL
    printf("Please wait. Initialising the program.\n\n"); //Displays a waiting message to user while curl options are set and request is performed to API server
    CURLcode res; //Basically all CURL functions return a code if they are executed successfully. We declare a variable res that stores the code and checks for error later on
    //setopt tells the curl how to behave. We set the appropriate options ourselves
    curl_easy_setopt(curl, CURLOPT_URL, test_url);  //CURLOPT URL sets the url for curl
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); //WRITEFUNCTION tells curl how to store the data received from server into the memory
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk); //WRITEDATA tells curl to write all the data in the chunk variable
    res = curl_easy_perform(curl); //Performs the curl operation (sending response to the server) and writes all the data into chunk.memory

    long response_code; //Stores response code received by the server. This is different form CURLcode res that is for CURL functions
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code); //Tells CURL to store the response code received by the server in response_code

    parsed_json = json_tokener_parse(chunk.memory); //As the data return to us by the server is in JSON format, we have to parse it
    json_object_object_get_ex(parsed_json, "route", &route); //Gets the "route" object that basically has all our route data from the JSON and writes it to a variable route
    json_object_object_get_ex(parsed_json, "info", &info); //Gets the "info" object that has the information about our server request
    json_object_object_get_ex(info, "statuscode", &statuscode); //Gets the "statuscode" object that has the HTTP server response value
    if (json_object_get_int(statuscode) == 0) //Checks if a request was successfully written to the API
    {
        struct json_object* results, * resultt; //Creates two JSON Objects to hold the results

        results = json_object_get_string(route); //Converts the route JSON object to a string and writes it to the result JSON object
        resultt = json_tokener_parse(results); //Parses the results json and stores it inside the resultt json object

        json_object_object_get_ex(resultt, "distance", &distance); //Gets the distance element from the result and stores it in distance
        if (strcmp(temp2, "k") == 0) //Checks if the user prefers km or miles
            printf("\nDistance between %s and %s is: %d km\n", temp, temp1, json_object_get_int(distance)); //Obtains the distance as an int and prints it
        else
            printf("Distance between %s and %s is: %d miles\n", temp, temp1, json_object_get_int(distance));

        json_object_object_get_ex(resultt, "formattedTime", &time); //Gets the formattedTime element from the resultt JSON object
        printf("Time taken to travel on average speed: %s\n", json_object_get_string(time)); //Obtains the time as a string and prints it
        json_object_object_get_ex(resultt, "fuelUsed", &fuel); //Gets the fuelUsed element from resultt and stores it in fuel
        if (strcmp(temp2, "k") == 0)
            printf("An approximate %.2f litres of fuel will be used\n", 3.785 * json_object_get_double(fuel)); //Obtains the fuel used as a double and prints it
        else
            printf("An approximate %.2f gallons of fuel will be used\n", json_object_get_double(fuel));
        printf("An estimated PKR %.2f is required to complete the road trip\n", 3.785 * json_object_get_double(fuel) * 109.25); //Uses the current national average cost for fuel prices to calculate the total price for the road trip

        printf("\nWould you like to view the route for the said road trip? (Y/N) "); //Asking user if he/she wants to view the road trip details
        fgets(temp4, 10, stdin); //Taking input for users decision
        temp4[strlen(temp4) - 1] = '\0';

        if (strcmp(temp4, "Y") == 0) //Checks if the user inputted y or Y, otherwise doesn't print them
        {
            puts("");
            struct json_object* maneuvers, * legs, * l, * l1, * lt, * element, * narratives; //Creates JSON objects to store data into
            json_object_object_get_ex(resultt, "legs", &legs); //Gets the "legs" name/value pair within the resultt JSON Object and stores its data into the legs object
            l = json_object_get_string(legs); //Converts the leg JSON Object into a string and stores it into the l object
            l1 = json_tokener_parse(l); //Parses the l object and stores the parsed data into the l1 object
            lt = json_object_array_get_idx(l1, 0); //The legs object was actually an array with only 1 index. So we copy all the data from the array's first and only index and write to the lt object
            json_object_object_get_ex(lt, "maneuvers", &maneuvers); //Searches the "maneuvers" name/value pair within the object and writes its data to the maneuvers object, which turns out to be an JSON Object Array

            int maneuvers_count = json_object_array_length(maneuvers); //Gets the array length of maneuvers and stores the value in maneuvers count
            for (int i = 0; i < maneuvers_count; i++) //Loops through all indexes of the maneuver array
            {
                element = json_object_array_get_idx(maneuvers, i); //Gets all the name/value pairs for the i'th index of maneuver array
                json_object_object_get_ex(element, "narrative", &narratives); //Gets the "narrative" named element inside that index
                printf("%d. %s\n", i + 1, json_object_get_string(narratives)); //Gets the value of the narrative in the form of a string and prints it out
            }
        }
        curl_easy_cleanup(curl); //Closes/ends the CURL handle and deletes everything from memory
    }

    else //If program does not run properly, it displays the error message sent by the API server and prints it for user to let them know
    {
        json_object_object_get_ex(info, "messages", &message); //Finds the messages key in the message JSON Object
        printf("Error %d\n", json_object_get_int(statuscode)); //Gets  the int value of the statuscode JSON object and prints it
        m_temp = json_object_array_get_idx(message, 0); //Copies all the contents of 0th index of message JSON array into m_temp
        printf("%s\n\n", json_object_get_string(m_temp)); //Gets the string value of m_temp JSON Object and prints it out
    }
    if (res != CURLE_OK) { //If result is not OK, error is printed. Note these variables are predefined in libcurl
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res)); //res which stores the result if curl executed properly. If it didn't, the error is printed using its function
    }
    else {

        printf("\n(%lu bytes retrieved)\n", (unsigned long)chunk.size); //An extra printf statement that tells how many bytes were received from the API
    }
    system("pause");
    free(chunk.memory); free(temp); free(temp1); free(temp2); free(temp3); free(temp4); //Freeing up the allocated memory to avoid memory leak
    curl_global_cleanup(); //Releases resources acquired since the inititialization of curl
}
