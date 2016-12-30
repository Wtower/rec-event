<?php
/**
 * @file
 *   Provide default view
 * 
 * @author
 *   gkarak@forecasting.gr
 *   
 * @copyright
 *   (c) 2014 George Karakostas
 */

/**
 * Implements hook_views_default_views().
 * 
 * To modify in module use hook_views_default_views_alter() 
 * function mymodule_views_default_views_alter(&$views) {
 *   $handler =& $views['event']->display['default']->handler; }
 */
function rec_event_views_default_views() {
  /*
   * Events view
   */
  $view = new view;
  $view->name = 'events';
  $view->description = 'Present a list of events.';
  $view->tag = 'rec_event';
  $view->base_table = 'rec_event';
  $view->human_name = 'Events';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE;

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Events';
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['query']['options']['query_comment'] = FALSE;
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '20';
  $handler->display->display_options['style_plugin'] = 'table';
  /* Header: Global: Text area */
  $handler->display->display_options['header']['area']['id'] = 'area';
  $handler->display->display_options['header']['area']['table'] = 'views';
  $handler->display->display_options['header']['area']['field'] = 'area';
  $handler->display->display_options['header']['area']['empty'] = FALSE;
  $handler->display->display_options['header']['area']['content'] = '<a href="<?php print url(\'event/add\'); ?>>Add Event</a>';
  $handler->display->display_options['header']['area']['format'] = 'full_html';
  $handler->display->display_options['header']['area']['tokenize'] = 0;
  /* Relationship: Author */
  $handler->display->display_options['relationships']['created_uid']['id'] = 'created_uid';
  $handler->display->display_options['relationships']['created_uid']['table'] = 'rec_event';
  $handler->display->display_options['relationships']['created_uid']['field'] = 'created_uid';
  $handler->display->display_options['relationships']['created_uid']['label'] = 'Author';
  $handler->display->display_options['relationships']['created_uid']['required'] = 0;
  /* Field: Event ID */
  $handler->display->display_options['fields']['event_id']['id'] = 'event_id';
  $handler->display->display_options['fields']['event_id']['table'] = 'rec_event';
  $handler->display->display_options['fields']['event_id']['field'] = 'event_id';
  /* Field: Type */
  $handler->display->display_options['fields']['type']['id'] = 'type';
  $handler->display->display_options['fields']['type']['table'] = 'rec_event';
  $handler->display->display_options['fields']['type']['field'] = 'type';
  $handler->display->display_options['fields']['type']['label'] = 'Type';
  /* Field: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'rec_event';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['title']['alter']['make_link'] = 1;
  $handler->display->display_options['fields']['title']['alter']['path'] = 'event/[event_id]';
  $handler->display->display_options['fields']['title']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['title']['alter']['external'] = 0;
  $handler->display->display_options['fields']['title']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['title']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['title']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['title']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['title']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['title']['alter']['html'] = 0;
  $handler->display->display_options['fields']['title']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['title']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['title']['hide_empty'] = 0;
  $handler->display->display_options['fields']['title']['empty_zero'] = 0;
  $handler->display->display_options['fields']['title']['hide_alter_empty'] = 1;
  /* Field: Event Date */
  $handler->display->display_options['fields']['event_date']['id'] = 'event_date';
  $handler->display->display_options['fields']['event_date']['table'] = 'field_data_event_date';
  $handler->display->display_options['fields']['event_date']['field'] = 'event_date';
  $handler->display->display_options['fields']['event_date']['settings'] = array(
    'format_type' => 'short',
    'fromto' => 'both',
    'multiple_number' => '',
    'multiple_from' => '',
    'multiple_to' => '',
  );
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['relationship'] = 'created_uid';
  $handler->display->display_options['fields']['name']['label'] = 'Author';
  $handler->display->display_options['fields']['name']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['name']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['name']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['name']['alter']['external'] = 0;
  $handler->display->display_options['fields']['name']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['name']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['name']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['name']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['name']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['name']['alter']['html'] = 0;
  $handler->display->display_options['fields']['name']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['name']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['name']['hide_empty'] = 0;
  $handler->display->display_options['fields']['name']['empty_zero'] = 0;
  $handler->display->display_options['fields']['name']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['name']['link_to_user'] = 1;
  $handler->display->display_options['fields']['name']['overwrite_anonymous'] = 0;
  $handler->display->display_options['fields']['name']['format_username'] = 1;
  /* Field: Date created */
  $handler->display->display_options['fields']['created_date']['id'] = 'created_date';
  $handler->display->display_options['fields']['created_date']['table'] = 'rec_event';
  $handler->display->display_options['fields']['created_date']['field'] = 'created_date';
  $handler->display->display_options['fields']['created_date']['label'] = 'Created';
  $handler->display->display_options['fields']['created_date']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['created_date']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['created_date']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['created_date']['alter']['external'] = 0;
  $handler->display->display_options['fields']['created_date']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['created_date']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['created_date']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['created_date']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['created_date']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['created_date']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['created_date']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['created_date']['alter']['html'] = 0;
  $handler->display->display_options['fields']['created_date']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['created_date']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['created_date']['hide_empty'] = 0;
  $handler->display->display_options['fields']['created_date']['empty_zero'] = 0;
  $handler->display->display_options['fields']['created_date']['hide_alter_empty'] = 1;
  $handler->display->display_options['fields']['created_date']['date_format'] = 'short';
  /* Field: Global: Custom text */
  $handler->display->display_options['fields']['nothing']['id'] = 'nothing';
  $handler->display->display_options['fields']['nothing']['table'] = 'views';
  $handler->display->display_options['fields']['nothing']['field'] = 'nothing';
  $handler->display->display_options['fields']['nothing']['label'] = 'Edit';
  $handler->display->display_options['fields']['nothing']['alter']['text'] = 'Edit';
  $handler->display->display_options['fields']['nothing']['alter']['make_link'] = 1;
  $handler->display->display_options['fields']['nothing']['alter']['path'] = 'event/[event_id]/edit';
  $handler->display->display_options['fields']['nothing']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['nothing']['alter']['external'] = 0;
  $handler->display->display_options['fields']['nothing']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['nothing']['alter']['trim_whitespace'] = 0;
  $handler->display->display_options['fields']['nothing']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['nothing']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['nothing']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['nothing']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['nothing']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['nothing']['alter']['html'] = 0;
  $handler->display->display_options['fields']['nothing']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['nothing']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['nothing']['hide_empty'] = 0;
  $handler->display->display_options['fields']['nothing']['empty_zero'] = 0;
  $handler->display->display_options['fields']['nothing']['hide_alter_empty'] = 0;

  // enable to allow operations field
  /* Field: Global: Custom text */
  //$handler->display->display_options['fields']['nothing']['exclude'] = TRUE;
  /* Field: Global: Custom text */
  /*$handler->display->display_options['fields']['nothing_1']['id'] = 'nothing_1';
  $handler->display->display_options['fields']['nothing_1']['table'] = 'views';
  $handler->display->display_options['fields']['nothing_1']['field'] = 'nothing';
  $handler->display->display_options['fields']['nothing_1']['label'] = 'Delete';
  $handler->display->display_options['fields']['nothing_1']['exclude'] = TRUE;
  $handler->display->display_options['fields']['nothing_1']['alter']['text'] = 'Delete';
  $handler->display->display_options['fields']['nothing_1']['alter']['make_link'] = TRUE;
  $handler->display->display_options['fields']['nothing_1']['alter']['path'] = 'event/[event_id]/delete';
  /* Field: Global: Custom text */
  /*$handler->display->display_options['fields']['nothing_2']['id'] = 'nothing_3';
  $handler->display->display_options['fields']['nothing_2']['table'] = 'views';
  $handler->display->display_options['fields']['nothing_2']['field'] = 'nothing';
  $handler->display->display_options['fields']['nothing_2']['label'] = 'Operations';
  $handler->display->display_options['fields']['nothing_2']['alter']['text'] =
    '<ul class="links inline">
      <li>[nothing]</li>
      <li>[nothing_1]</li>
    </ul>';*/
  
  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['path'] = 'event';
  $handler->display->display_options['menu']['type'] = 'normal';
  $handler->display->display_options['menu']['title'] = 'Events';
  $handler->display->display_options['menu']['weight'] = '0';
  
  $views['event'] = $view;
  
  /*
   * Calendar View
   */
  if (module_exists('fullcalendar')) {
    $view = new view();
    $view->name = 'calendar';
    $view->description = 'Display a calendar of events';
    $view->tag = 'rec_event';
    $view->base_table = 'rec_event';
    $view->human_name = 'Calendar';
    $view->core = 7;
    $view->api_version = '3.0';
    $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
    
    /* Display: Master */
    $handler = $view->new_display('default', 'Master', 'default');
    $handler->display->display_options['title'] = 'Calendar';
    $handler->display->display_options['use_more_always'] = FALSE;
    $handler->display->display_options['use_more_text'] = 'περισσότερα';
    $handler->display->display_options['access']['type'] = 'none';
    $handler->display->display_options['cache']['type'] = 'none';
    $handler->display->display_options['query']['type'] = 'views_query';
    $handler->display->display_options['exposed_form']['type'] = 'basic';
    $handler->display->display_options['pager']['type'] = 'none';
    $handler->display->display_options['style_plugin'] = 'fullcalendar';
    $handler->display->display_options['style_options']['firstDay'] = '1';
    $handler->display->display_options['style_options']['advanced'] = TRUE;
    $handler->display->display_options['style_options']['titleformatWeek'] = 'd [MMM ][yyyy]{ - d MMM yyyy}';
    $handler->display->display_options['style_options']['columnformatWeek'] = 'ddd d/M';
    $handler->display->display_options['style_options']['titleformatDay'] = 'dddd, d MMM yyyy';
    $handler->display->display_options['style_options']['columnformatDay'] = 'dddd d/M';
    $handler->display->display_options['style_options']['sameWindow'] = TRUE;
    $handler->display->display_options['style_options']['fullcalendar_options']['defaultEventMinutes'] = 30;
    /* Field: ID */
    $handler->display->display_options['fields']['event_id']['id'] = 'event_id';
    $handler->display->display_options['fields']['event_id']['table'] = 'rec_event';
    $handler->display->display_options['fields']['event_id']['field'] = 'event_id';
    /* Field: Title */
    $handler->display->display_options['fields']['title']['id'] = 'title';
    $handler->display->display_options['fields']['title']['table'] = 'rec_event';
    $handler->display->display_options['fields']['title']['field'] = 'title';
    $handler->display->display_options['fields']['title']['label'] = 'Title';
    /* Field: Date */
    $handler->display->display_options['fields']['event_date']['id'] = 'event_date';
    $handler->display->display_options['fields']['event_date']['table'] = 'field_data_event_date';
    $handler->display->display_options['fields']['event_date']['field'] = 'event_date';
    $handler->display->display_options['fields']['event_date']['label'] = 'Event Date';
    
    /* Display: Page */
    $handler = $view->new_display('page', 'Page', 'page');
    $handler->display->display_options['path'] = 'calendar';
    $handler->display->display_options['menu']['type'] = 'normal';
    $handler->display->display_options['menu']['title'] = 'Calendar';
    $handler->display->display_options['menu']['weight'] = '0';
    $handler->display->display_options['menu']['context'] = 0;
    $handler->display->display_options['menu']['context_only_inline'] = 0;
    
    $views['calendar'] = $view;
  }

  return $views;
}