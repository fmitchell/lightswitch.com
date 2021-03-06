<?php

/**
 * FORMS
 */

/**
 * Form submit: client terms
 */
function bxdev_document_client_terms_form_submit($form, &$form_state){
	
	$nid = arg(2);
	$project = new Project($nid);
	$data['project'] = $project;
	
	// get the client service agreement html
	$content = variable_get('document_client_service_agreement', array());
	
	$html = token_replace($content['value'], $data);
	
	$client_name = bxdev_get_filename_string($project->client->field_name['und'][0]['value']);
	
	$filename = 'lightswitch_client_service_agreement_' . $client_name . '_' . $project->project->nid . '.pdf';
	
	$fid = bxdev_document_create_pdf($html, $filename, 'client', 'service_agreements', 'Client Service Agreement', $project->client->uid, $project->project->nid);
	
	if($fid){
		$project->send_client_service_agreement($fid);
		
		// check for redirect
		$redirect = arg(4);
		if(!empty($redirect)){
			drupal_goto('node/' . $nid);
		}
		
		drupal_goto('node/1419');
	}
	
}

/**
 * Form builder: document settings
 */
function bxdev_document_settings_form($form, &$form_state){
		
	$client_service_agreement = variable_get('document_client_service_agreement', array());
	
	$form['client_service_agreement_fieldset'] = array(
		'#type' => 'fieldset',
		'#title' => t('Client Service Agreement'),
		'#collapsible' => TRUE,
		'#collapsed' => FALSE,
	);
	
	$form['client_service_agreement_fieldset']['document_client_service_agreement'] = array(
		'#type' => 'text_format',
		'#default_value' => !empty($client_service_agreement) ? $client_service_agreement['value'] : '',
	);
	
	$videographer_independent_contractor_agreement = variable_get('document_videographer_independent_contractor_agreement', array());
	
	$form['videographer_independent_contractor_agreement_fieldset'] = array(
		'#type' => 'fieldset',
		'#title' => t('Videographer Independent Contractor Agreement'),
		'#collapsible' => TRUE,
		'#collapsed' => FALSE,
	);
	
	$form['videographer_independent_contractor_agreement_fieldset']['document_videographer_independent_contractor_agreement'] = array(
		'#type' => 'text_format',
		'#default_value' => !empty($videographer_independent_contractor_agreement) ? $videographer_independent_contractor_agreement['value'] : '',
	);	
	
	return system_settings_form($form);
}

/**
 * Form submit: videographer user signup
 */
function bxdev_document_videographer_user_register_submit($form, &$form_state){
	
	$account = $form_state['user'];
	$profile = profile2_load_by_user($account, 'videographer');
	
	$data['full_name'] = $account->field_name['und'][0]['value'];
	$data['address'] = $profile->field_address['und'][0]['value'];
	
	$html = variable_get('document_videographer_independent_contractor_agreement', '');
	$html = $html['value'];
	$html = token_replace($html, $data);
	
	$videographer_name = bxdev_get_filename_string($account->field_name['und'][0]['value']);
	
	$filename = 'lightswitch_contractor_agreement_' . $videographer_name . '_' . $account->uid . '.pdf';
	$fid = bxdev_document_create_pdf($html, $filename, 'videographer', 'independent_contractor_agreements', 'Independent Contractor Agreement', $account->uid);
	
	// redirect
	$form_state['redirect'] = 'node/1421';
	
}
