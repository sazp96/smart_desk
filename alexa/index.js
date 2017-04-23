'use strict';

// --------------- Helpers that build all of the responses -----------------------

function buildSpeechletResponse(title, output, repromptText, shouldEndSession) {
    return {
        outputSpeech: {
            type: 'PlainText',
            text: output,
        },
        card: {
            type: 'Simple',
            title: `Smart Desk`,
            content: `${output}`,
        },
        reprompt: {
            outputSpeech: {
                type: 'PlainText',
                text: repromptText,
            },
        },
        shouldEndSession,
    };
}

function buildResponse(sessionAttributes, speechletResponse) {
    return {
        version: '1.0',
        sessionAttributes,
        response: speechletResponse,
    };
}


// --------------- Functions that control the skill's behavior -----------------------

function getWelcomeResponse(callback) {
    // If we wanted to initialize the session to have some attributes we could add those here.
    const sessionAttributes = {};
    const cardTitle = 'Welcome';
    const speechOutput = 'Welcome to your smart desk. You can tell me to sit, stand, set a standing target, or ask how am I doing.';
    // If the user either does not reply to the welcome message or says something that is not
    // understood, they will be prompted again with this text.
    const repromptText = 'You can tell me to sit, stand, set a standing target, or ask how am I doing.';
    const shouldEndSession = false;

    callback(sessionAttributes,
        buildSpeechletResponse(cardTitle, speechOutput, repromptText, shouldEndSession));
}

function handleSessionEndRequest(callback) {
    const cardTitle = 'Session Ended';
    const speechOutput = 'Thanks for using your smart desk. Have a nice day!';
    // Setting this to true ends the session and exits the skill.
    const shouldEndSession = true;

    callback({}, buildSpeechletResponse(cardTitle, speechOutput, null, shouldEndSession));
}


/**
 * Sets the desk to stand or sit, returns progress, and sets target
 */
 
function setDeskToStand(intent, session, callback) {
    console.log('setDeskToStand');
    const cardTitle = intent.name;
    let sessionAttributes = {};
    let speechOutput = 'Setting desk to stand';
    let repromptText = 'You can raise your desk by saying, tell smart desk to stand';
    
    let request = require('request');
    // Need to pass user's actual device id and access token in DEVICE_ID and ACCESS_TOKEN below
    request.post({
        url:     'https://api.particle.io/v1/devices/DEVICE_ID/changeMode',
        form:    { access_token: "ACCESS_TOKEN", args: "stand" }
    }, function(error, response, body){
        console.log(body);
    });
    const shouldEndSession = true;

    callback(sessionAttributes, buildSpeechletResponse(cardTitle, speechOutput, repromptText, shouldEndSession));
}

function setDeskToSit(intent, session, callback) {
    console.log('setDeskToStand');
    const cardTitle = intent.name;
    let sessionAttributes = {};
    let speechOutput = 'Setting desk to sit';
    let repromptText = 'You can lower your desk by saying, tell smart desk to sit';

    let request = require('request');
    // Need to pass user's actual device id and access token in DEVICE_ID and ACCESS_TOKEN below
    request.post({
      url:     'https://api.particle.io/v1/devices/DEVICE_ID/changeMode',
      form:    { access_token: "ACCESS_TOKEN", args: "sit" }
    }, function(error, response, body){
        console.log(body);
    });
    const shouldEndSession = true;

    callback(sessionAttributes, buildSpeechletResponse(cardTitle, speechOutput, repromptText, shouldEndSession));
}

function setTarget(intent, session, callback) {
    console.log('setTarget');
    const cardTitle = intent.name;
    let sessionAttributes = {};
    let speechOutput = 'OK. Setting your standing target.';
    let repromptText = 'You can set your target by saying set my target.';

    let request = require('request');
    // Need to pass user's actual device id and access token in DEVICE_ID and ACCESS_TOKEN below
    request.post({
      url:     'https://api.particle.io/v1/devices/DEVICE_ID/setTarget',
      form:    { access_token: "ACCESS_TOKEN", args: "30" }
    }, function(error, response, body){
        console.log(body);
    });
    const shouldEndSession = true;

    callback(sessionAttributes, buildSpeechletResponse(cardTitle, speechOutput, repromptText, shouldEndSession));
}

function getProgress(intent, session, callback) {
    console.log('getProgress');
    const cardTitle = intent.name;
    let sessionAttributes = {};
    let speechOutput = 'Getting your progress. ';
    let repromptText = 'You can get your progress by saying how am I doing';

    let request = require('request');
    // Need to pass user's actual device id and access token in DEVICE_ID and ACCESS_TOKEN below
    request.get({
      url:     'https://api.particle.io/v1/devices/DEVICE_ID/statusMess?access_token=ACCESS_TOKEN'
    }, function(error, response, body){
	speechOutput += JSON.parse(body)['result'];
        console.log(speechOutput);
	const shouldEndSession = true;

	callback(sessionAttributes, buildSpeechletResponse(cardTitle, speechOutput, repromptText, shouldEndSession));    
    });
}


// --------------- Events -----------------------

/**
 * Called when the session starts.
 */
function onSessionStarted(sessionStartedRequest, session) {
    console.log(`onSessionStarted requestId=${sessionStartedRequest.requestId}, sessionId=${session.sessionId}`);
}

/**
 * Called when the user launches the skill without specifying what they want.
 */
function onLaunch(launchRequest, session, callback) {
    console.log(`onLaunch requestId=${launchRequest.requestId}, sessionId=${session.sessionId}`);

    // Dispatch to your skill's launch.
    getWelcomeResponse(callback);
}

/**
 * Called when the user specifies an intent for this skill.
 */
function onIntent(intentRequest, session, callback) {
    console.log(`onIntent requestId=${intentRequest.requestId}, sessionId=${session.sessionId}`);

    const intent = intentRequest.intent;
    const intentName = intentRequest.intent.name;

    console.log('parsing intent');

    // Dispatch to your skill's intent handlers
    if (intentName === 'StandIntent') {
        setDeskToStand(intent, session, callback);
    } else if (intentName === 'SitIntent') {
        setDeskToSit(intent, session, callback);
    } else if (intentName === 'SetTargetIntent') {
        setTarget(intent, session, callback);
    } else if (intentName === 'ProgressIntent') {
        getProgress(intent, session, callback);
    } else {
        throw new Error('Invalid intent');
    }
}

/**
 * Called when the user ends the session.
 * Is not called when the skill returns shouldEndSession=true.
 */
function onSessionEnded(sessionEndedRequest, session) {
    console.log(`onSessionEnded requestId=${sessionEndedRequest.requestId}, sessionId=${session.sessionId}`);
    // Add cleanup logic here
}


// --------------- Main handler -----------------------

// Route the incoming request based on type (LaunchRequest, IntentRequest,
// etc.) The JSON body of the request is provided in the event parameter.
exports.handler = (event, context, callback) => {
    try {
        console.log(`event.session.application.applicationId=${event.session.application.applicationId}`);

        /**
         * Uncomment this if statement and populate with your skill's application ID to
         * prevent someone else from configuring a skill that sends requests to this function.
         */
        /*
        if (event.session.application.applicationId !== 'amzn1.echo-sdk-ams.app.[unique-value-here]') {
             callback('Invalid Application ID');
        }
        */

        if (event.session.new) {
            onSessionStarted({ requestId: event.request.requestId }, event.session);
        }

        if (event.request.type === 'LaunchRequest') {
            onLaunch(event.request,
                event.session,
                (sessionAttributes, speechletResponse) => {
                    callback(null, buildResponse(sessionAttributes, speechletResponse));
                });
        } else if (event.request.type === 'IntentRequest') {
            onIntent(event.request,
                event.session,
                (sessionAttributes, speechletResponse) => {
                    callback(null, buildResponse(sessionAttributes, speechletResponse));
                });
        } else if (event.request.type === 'SessionEndedRequest') {
            onSessionEnded(event.request, event.session);
            callback();
        }
    } catch (err) {
        callback(err);
    }
};
